#include "MainController.h"
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QtConcurrent/QtConcurrent>
#include <unordered_set>
#include <QSet>

// 正确的头文件路径（根据用户提示）
#include "../common/Logger.h"
#include "../common/WordEntry.h"
#include "../core/database/WordDatabaseManager.h"
#include "../core/dictionary/LookupService.h"
#include "../core/extractor/WordExtractor.h"
#include "../modules/import/FileImporter.h"
#include "../modules/ocr/OcrManager.h"
#include "../modules/export/ExportManager.h"
// 暂时注释掉数据库管理窗口（因为 ui/database 下的是空实现）
// #include "./database/DatabaseManagementWindow.h"

// 辅助函数：将 QStringList 转为 QVariantList
static QVariantList stringListToVariantList(const QStringList &list)
{
    QVariantList result;
    for (const QString &s : list)
        result.append(s);
    return result;
}

// 辅助函数：将 QList<WordEntry> 转为 QVariantList (每个元素是 QVariantMap)
static QVariantList wordEntryListToVariantList(const QList<WordEntry> &list)
{
    QVariantList result;
    for (const WordEntry &entry : list) {
        QVariantMap map;
        map["word"] = entry.word;
        map["translation"] = entry.translation;
        map["phonetic"] = entry.phonetic;
        result.append(map);
    }
    return result;
}

MainController::MainController(QObject *parent)
    : QObject(parent)
{
    initBackend();
    connect(&m_extractWatcher, &QFutureWatcher<void>::finished,
            this, &MainController::onExtractWordsFinished);
}

MainController::~MainController()
{
    if (m_extractWatcher.isRunning())
        m_extractWatcher.waitForFinished();
}

void MainController::initBackend()
{
    m_dbManager = new WordDatabaseManager("euwe_words.db", "Words", QString(), this);
    if (!m_dbManager->isOpen()) {
        LOG_ERROR_CAT(LogCategory::Database) << "WordDatabaseManager 打开失败";
        setStatusMessage("数据库打开失败，部分功能不可用");
    } else {
        LOG_INFO_CAT(LogCategory::Database) << "熟词库初始化成功";
    }

    if (m_dbManager && m_dbManager->isOpen()) {
        m_lookupService = new LookupService(m_dbManager->database(), this);
    } else {
        LOG_WARNING_CAT(LogCategory::Database) << "词典服务不可用";
    }

    m_ocrManager = new OcrManager(this);
    connect(m_ocrManager, &OcrManager::finished, this, &MainController::onOcrFinished);
}

void MainController::setStatusMessage(const QString &msg)
{
    if (m_statusMessage != msg) {
        m_statusMessage = msg;
        emit statusMessageChanged();
    }
}

void MainController::setProcessing(bool processing)
{
    if (m_isProcessing != processing) {
        m_isProcessing = processing;
        emit processingChanged();
    }
}

void MainController::classifyWords(const std::unordered_set<QString> &extractedWords)
{
    if (extractedWords.empty()) {
        m_knownWords.clear();
        m_newWords.clear();
        m_unknownWords.clear();
        updateWordStats();
        setStatusMessage("文本中未发现英语单词");
        return;
    }

    QSet<QString> knownSet;
    if (m_dbManager && m_dbManager->isOpen()) {
        QList<WordEntry> allKnown = m_dbManager->getAllWords();
        for (const WordEntry &w : allKnown)
            knownSet.insert(w.word);
    }

    QStringList localKnown;
    QList<WordEntry> localNew;
    QStringList localUnknown;

    for (const QString &w : extractedWords) {
        if (knownSet.contains(w)) {
            localKnown.append(w);
            continue;
        }
        WordEntry dictEntry;
        if (m_lookupService) {
            dictEntry = m_lookupService->lookupWord(w);
        }
        if (!dictEntry.word.isEmpty()) {
            localNew.append(dictEntry);
        } else {
            localUnknown.append(w);
        }
    }

    m_knownWords = localKnown;
    m_newWords = localNew;
    m_unknownWords = localUnknown;

    updateWordStats();
    setStatusMessage(QString("分析完成 - 熟词: %1, 生词: %2, 未识别: %3")
                         .arg(m_knownWords.size()).arg(m_newWords.size()).arg(m_unknownWords.size()));
    emit extractionFinished();
}

void MainController::updateWordStats()
{
    emit wordStatsChanged();
}

void MainController::extractWords(const QString &text)
{
    if (text.trimmed().isEmpty()) {
        setStatusMessage("输入文本为空，无法提取");
        return;
    }
    if (m_isProcessing) {
        setStatusMessage("正在处理其他任务，请稍后再试");
        return;
    }

    setProcessing(true);
    setStatusMessage("正在提取单词并分析...");

    WordDatabaseManager *dbMgr = m_dbManager;
    LookupService *lookup = m_lookupService;

    QFuture<void> future = QtConcurrent::run([this, text, dbMgr, lookup]() {
        WordExtractor extractor(text, dbMgr);
        auto wordsSet = extractor.getWords();
        QMetaObject::invokeMethod(this, [this, wordsSet]() {
            classifyWords(wordsSet);
            setProcessing(false);
        });
    });
    m_extractWatcher.setFuture(future);
}

void MainController::importFile(const QString &filePath)
{
    if (filePath.isEmpty()) return;
    if (m_isProcessing) {
        setStatusMessage("正在处理其他任务，请稍后再试");
        return;
    }

    setProcessing(true);
    setStatusMessage("正在导入文件...");

    QtConcurrent::run([this, filePath]() {
        ImportResult result = FileImporter::importFile(filePath);
        if (!result.text.isEmpty()) {
            emit fileImportFinished(result.text, result.imagePaths);
        }
        if (!result.imagePaths.isEmpty()) {
            m_ocrManager->startRecognition(result.imagePaths, "\n---\n");
            QMetaObject::invokeMethod(this, [this]() {
                setStatusMessage("文件导入成功，文本已更新，正在识别图片...");
            });
        } else {
            QMetaObject::invokeMethod(this, [this]() {
                setStatusMessage("文件导入成功，文本已更新");
                setProcessing(false);
            });
        }
        if (result.text.isEmpty() && result.imagePaths.isEmpty()) {
            QMetaObject::invokeMethod(this, [this]() {
                setStatusMessage("文件导入失败，未提取到任何内容");
                emit operationFailed("无法读取文件内容");
                setProcessing(false);
            });
        }
    });
}

void MainController::selectAndOcrImages()
{
    QStringList imagePaths = QFileDialog::getOpenFileNames(
        nullptr, "选择要识别的图片", QString(),
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.tiff *.gif);;所有文件 (*.*)");
    if (imagePaths.isEmpty()) return;

    if (m_isProcessing) {
        setStatusMessage("正在处理其他任务，请稍后再试");
        return;
    }

    setProcessing(true);
    setStatusMessage(QString("正在识别 %1 张图片...").arg(imagePaths.size()));
    m_ocrManager->startRecognition(imagePaths, "\n---\n");
}

void MainController::onOcrFinished(const QString &mergedText)
{
    emit ocrFinished(mergedText);
    setStatusMessage("OCR 识别完成");
    setProcessing(false);
}

void MainController::openDatabaseManager()
{
    // 由于 ui/database/DatabaseManagementWindow 是空实现，暂时显示消息框
    QMessageBox::information(nullptr, "提示", "数据库管理功能暂未实现（占位）");
    // 若将来实现，可取消注释以下代码并确保正确的头文件
    /*
    if (!m_dbManager || !m_dbManager->isOpen()) {
        QMessageBox::warning(nullptr, "错误", "数据库未打开，无法管理熟词库");
        return;
    }
    DatabaseManagementWindow *win = new DatabaseManagementWindow(m_dbManager->database(), nullptr);
    win->setAttribute(Qt::WA_DeleteOnClose);
    win->show();
    */
}

void MainController::clearText()
{
    setStatusMessage("文本已清空");
}

void MainController::copyToClipboard(const QString &text)
{
    if (text.isEmpty()) {
        setStatusMessage("没有文本可复制");
        return;
    }
    QApplication::clipboard()->setText(text);
    setStatusMessage("已复制到剪贴板");
}

void MainController::showHelp()
{
    QMessageBox::about(nullptr, "帮助",
                       "联系 Bcaid\nQQ:75011847\nWX:YBcaid\n\n"
                       "功能说明：\n"
                       "- 提取单词：从左侧文本提取英语单词，并自动分类\n"
                       "- 文件导入：支持 txt/pdf/docx/pptx 等，自动提取文本和图片 OCR\n"
                       "- 图片 OCR：手动选择图片识别文字\n"
                       "- 表格操作：选中生词或熟词，可录入/移出熟词库，导出生词表\n");
}

void MainController::showAbout()
{
    QMessageBox::about(nullptr, "关于",
                       "智能文本提取工具 EUWE\n"
                       "基于 Qt 6.8.3\n"
                       "版权所有 © Bcaid");
}

void MainController::addToKnown(const QVariantList &selectedIndexes)
{
    if (!m_dbManager) {
        setStatusMessage("熟词库管理器未就绪");
        return;
    }
    QList<WordEntry> toAdd;
    for (const QVariant &idxVar : selectedIndexes) {
        int row = idxVar.toInt();
        if (row >= 0 && row < m_newWords.size()) {
            toAdd.append(m_newWords[row]);
        }
    }
    if (toAdd.isEmpty()) {
        setStatusMessage("未选中任何生词");
        return;
    }

    int inserted = m_dbManager->addWords(toAdd);
    if (inserted > 0) {
        // 从生词列表移除，添加到熟词列表
        QSet<QString> addedWords;
        for (const WordEntry &e : toAdd)
            addedWords.insert(e.word);
        for (int i = m_newWords.size() - 1; i >= 0; --i) {
            if (addedWords.contains(m_newWords[i].word)) {
                m_knownWords.append(m_newWords[i].word);
                m_newWords.removeAt(i);
            }
        }
        updateWordStats();
        emit extractionFinished();
        setStatusMessage(QString("成功录入 %1 个单词到熟词库").arg(inserted));
    } else {
        setStatusMessage("录入失败，可能单词已存在或数据库错误");
    }
}

void MainController::removeFromKnown(const QVariantList &selectedIndexes)
{
    if (!m_dbManager) {
        setStatusMessage("熟词库管理器未就绪");
        return;
    }
    QStringList toRemove;
    for (const QVariant &idxVar : selectedIndexes) {
        int row = idxVar.toInt();
        if (row >= 0 && row < m_knownWords.size()) {
            toRemove << m_knownWords[row];
        }
    }
    if (toRemove.isEmpty()) {
        setStatusMessage("未选中任何熟词");
        return;
    }

    int deleted = m_dbManager->removeWords(toRemove);
    if (deleted > 0) {
        for (const QString &word : toRemove) {
            int idx = m_knownWords.indexOf(word);
            if (idx != -1) {
                m_knownWords.removeAt(idx);
                m_unknownWords.append(word);
            }
        }
        updateWordStats();
        emit extractionFinished();
        setStatusMessage(QString("成功从熟词库移出 %1 个单词").arg(deleted));
    } else {
        setStatusMessage("移出失败");
    }
}

void MainController::exportCsv()
{
    if (m_newWords.isEmpty()) {
        setStatusMessage("没有生词可导出");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(nullptr, "导出生词表 CSV",
                                                    QDir::homePath() + "/生词表.csv",
                                                    "CSV文件 (*.csv)");
    if (fileName.isEmpty()) return;
    bool ok = ExportManager::exportToCsv(m_newWords, fileName);
    if (ok) {
        setStatusMessage(QString("已导出 CSV 到 %1").arg(fileName));
    } else {
        setStatusMessage("CSV 导出失败");
    }
}

void MainController::exportPdf()
{
    if (m_newWords.isEmpty()) {
        setStatusMessage("没有生词可导出");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(nullptr, "导出生词表 PDF",
                                                    QDir::homePath() + "/生词表.pdf",
                                                    "PDF文件 (*.pdf)");
    if (fileName.isEmpty()) return;
    bool ok = ExportManager::exportToPdf(m_newWords, fileName);
    if (ok) {
        setStatusMessage(QString("已导出 PDF 到 %1").arg(fileName));
    } else {
        setStatusMessage("PDF 导出失败");
    }
}

void MainController::exportDoc()
{
    if (m_newWords.isEmpty()) {
        setStatusMessage("没有生词可导出");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(nullptr, "导出生词表 Word",
                                                    QDir::homePath() + "/生词表.doc",
                                                    "Word文档 (*.doc)");
    if (fileName.isEmpty()) return;
    bool ok = ExportManager::exportToDoc(m_newWords, fileName);
    if (ok) {
        setStatusMessage(QString("已导出 DOC 到 %1").arg(fileName));
    } else {
        setStatusMessage("DOC 导出失败");
    }
}

QVariantList MainController::knownWordsList() const
{
    return stringListToVariantList(m_knownWords);
}

QVariantList MainController::newWordsList() const
{
    return wordEntryListToVariantList(m_newWords);
}

QVariantList MainController::unknownWordsList() const
{
    return stringListToVariantList(m_unknownWords);
}

void MainController::onExtractWordsFinished()
{
    setProcessing(false);
}