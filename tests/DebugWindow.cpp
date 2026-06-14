#include "DebugWindow.h"
#include "DebugTool.h"
#include "../core/extractor/WordExtractor.h"
#include "../core/database/WordDatabaseManager.h"
#include "../modules/import/FileImporter.h"
#include "../modules/ocr/OcrManager.h"
#include "../modules/export/ExportManager.h"
#include "../common/TextUtils.h"
#include "../common/WordEntry.h"
#include "../common/Logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QRegularExpression>
#include <QFontMetrics>
#include <QSqlQuery>
#include <QDateTime>
#include <QScrollBar>
#include <QSizePolicy>
#include <QFile>
#include <QTextStream>

DebugWindow::DebugWindow(QWidget *parent)
    : QWidget(parent)
{
    m_dbManager = new WordDatabaseManager("euwe_words.db", "Words", QString(), this);
    m_lookupService = new LookupService(m_dbManager->database(), this);

    m_ocrManager = new OcrManager(this);
    m_ocrManager->setMaxThreads(4);
    connect(m_ocrManager, &OcrManager::finished, this, &DebugWindow::onOcrFinished);

    setupUI();
    updateStats();
    m_statusLabel->setText("✨ 就绪 - 可以开始提取文本");

    QSqlQuery query(m_dbManager->database());
    if (query.exec("SELECT word FROM stardict LIMIT 5")) {
        int cnt = 0;
        while (query.next()) cnt++;
        log(QString("数据库连接成功，stardict 表前5行数据条数: %1").arg(cnt));
    } else {
        log("数据库连接警告: stardict 表可能为空或不存在");
    }
    log("调试窗口已启动");
}

DebugWindow::~DebugWindow() {}

void DebugWindow::log(const QString &msg)
{
    if (m_debugOutput) {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        m_debugOutput->appendPlainText(timestamp + " " + msg);
        QScrollBar *bar = m_debugOutput->verticalScrollBar();
        bar->setValue(bar->maximum());
    }
}

void DebugWindow::setupUI()
{
    setWindowTitle("EUWE 全功能测试窗口(DEBUG Tool)");
    setMinimumSize(1100, 650);
    setStyleSheet(
        "QWidget { background-color: #f0f2f5; font-family: 'Microsoft YaHei', 'Segoe UI', sans-serif; }"
        "QTextEdit, QPlainTextEdit, QTableWidget { background-color: white; border: 1px solid #d0d7de; border-radius: 8px; }"
        "QTextEdit:focus, QTableWidget:focus { border: 1px solid #409eff; }"
        "QPlainTextEdit { font-family: 'JetBrains Mono', 'Consolas', monospace; font-size: 11px; background-color: #1e1e1e; color: #d4d4d4; }"
        "QPushButton { border: none; border-radius: 6px; font-weight: bold; font-size: 12px; color: white; padding: 6px 12px; }"
        "QPushButton:hover { transform: translateY(-1px); box-shadow: 0 2px 8px rgba(0,0,0,0.15); }"
        "QPushButton:pressed { transform: translateY(0px); }"
        "QLabel { color: #2c3e50; font-size: 12px; font-weight: 500; }"
        "QGroupBox { font-weight: bold; font-size: 13px; border: 1px solid #e2e8f0; border-radius: 8px; margin-top: 8px; padding-top: 8px; background-color: white; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; background-color: white; }"
        "QHeaderView::section {"
        "   background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f6f8fa, stop:1 #e9ecef);"
        "   color: #1f2d3d;"
        "   font-weight: bold;"
        "   font-size: 12px;"
        "   padding: 6px 8px;"
        "   border: none;"
        "   border-right: 1px solid #e2e8f0;"
        "   border-bottom: 1px solid #e2e8f0;"
        "}"
        "QHeaderView::section:last { border-right: none; }"
        "QHeaderView::section:hover { background-color: #e2e8f0; }"
        "QTableWidget::item { padding: 4px; border-bottom: 1px solid #f0f0f0; }"
        "QTableWidget::item:selected { background-color: #e3f2fd; color: #0d47a1; }"
        "QTableWidget { alternate-background-color: #fafbfc; }"
        "QTableWidget { gridline-color: #e9ecef; }"
        );

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    QWidget *leftPanel = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(4);

    QLabel *titleLabel = new QLabel("📝 待提取文本", leftPanel);
    titleLabel->setStyleSheet("font-size: 13px; font-weight: bold; margin-bottom: 2px; color: #1f2d3d;");
    leftLayout->addWidget(titleLabel);

    m_textEdit = new QTextEdit(leftPanel);
    m_textEdit->setMinimumHeight(220);
    m_textEdit->setPlaceholderText("在此输入文本，或点击下方按钮导入文件/图片...");
    connect(m_textEdit, &QTextEdit::textChanged, this, &DebugWindow::onTextChanged);
    leftLayout->addWidget(m_textEdit);

    QGroupBox *opGroup = new QGroupBox("⚡ 操作", leftPanel);
    QGridLayout *btnLayout = new QGridLayout(opGroup);
    btnLayout->setSpacing(6);
    btnLayout->setContentsMargins(8, 8, 8, 8);

    QPushButton *extractBtn = new QPushButton("📋 提取单词", opGroup);
    extractBtn->setStyleSheet("background-color: #ed8936;");
    connect(extractBtn, &QPushButton::clicked, this, &DebugWindow::onExtractWords);

    QPushButton *importBtn = new QPushButton("📁 文件导入", opGroup);
    importBtn->setStyleSheet("background-color: #48bb78;");
    connect(importBtn, &QPushButton::clicked, this, &DebugWindow::onImportFile);

    QPushButton *ocrBtn = new QPushButton("🖼️ 图片OCR", opGroup);
    ocrBtn->setStyleSheet("background-color: #9f7aea;");
    connect(ocrBtn, &QPushButton::clicked, this, &DebugWindow::onOcrImages);

    QPushButton *clearBtn = new QPushButton("🗑️ 清空", opGroup);
    clearBtn->setStyleSheet("background-color: #cbd5e0; color: #2d3748;");
    connect(clearBtn, &QPushButton::clicked, this, &DebugWindow::onClearText);

    QPushButton *copyBtn = new QPushButton("📋 复制", opGroup);
    copyBtn->setStyleSheet("background-color: #cbd5e0; color: #2d3748;");
    connect(copyBtn, &QPushButton::clicked, this, &DebugWindow::onCopyText);

    QPushButton *helpBtn = new QPushButton("❓ 帮助", opGroup);
    helpBtn->setStyleSheet("background-color: #4299e1;");
    connect(helpBtn, &QPushButton::clicked, this, &DebugWindow::onHelp);

    QPushButton *aboutBtn = new QPushButton("ℹ️ 关于", opGroup);
    aboutBtn->setStyleSheet("background-color: #667eea;");
    connect(aboutBtn, &QPushButton::clicked, this, &DebugWindow::onAbout);

    btnLayout->addWidget(extractBtn, 0, 0);
    btnLayout->addWidget(importBtn, 0, 1);
    btnLayout->addWidget(ocrBtn, 0, 2);
    btnLayout->addWidget(clearBtn, 1, 0);
    btnLayout->addWidget(copyBtn, 1, 1);
    btnLayout->addWidget(helpBtn, 1, 2);
    btnLayout->addWidget(aboutBtn, 2, 0, 1, 3);
    leftLayout->addWidget(opGroup);

    m_statusLabel = new QLabel("✨ 就绪", leftPanel);
    m_statusLabel->setStyleSheet("color: #718096; padding: 2px;");
    m_statusLabel->setMaximumHeight(100);
    leftLayout->addWidget(m_statusLabel);

    QLabel *debugLabel = new QLabel("🐞 调试输出", leftPanel);
    debugLabel->setStyleSheet("font-weight: bold; margin-top: 4px; color: #1f2d3d;");
    leftLayout->addWidget(debugLabel);

    m_debugOutput = new QPlainTextEdit(leftPanel);
    m_debugOutput->setReadOnly(true);
    m_debugOutput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_debugOutput->setFont(QFont("JetBrains Mono", 9));
    m_debugOutput->setStyleSheet("background-color: #1e1e1e; color: #d4d4d4; border: 1px solid #333;");
    leftLayout->addWidget(m_debugOutput);

    QLabel *cmdLabel = new QLabel("💻 命令行", leftPanel);
    cmdLabel->setStyleSheet("font-weight: bold; margin-top: 8px; color: #1f2d3d;");
    leftLayout->addWidget(cmdLabel);

    m_commandInput = new QLineEdit(leftPanel);
    m_commandInput->setPlaceholderText("输入命令，按回车执行 (例如: checkDir, exportCode)");
    m_commandInput->setStyleSheet("padding: 6px; border: 1px solid #d0d7de; border-radius: 6px;");
    connect(m_commandInput, &QLineEdit::returnPressed, this, &DebugWindow::onCommandEntered);
    leftLayout->addWidget(m_commandInput);

    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(4);

    QGroupBox *unknownGroup = new QGroupBox("❓ 未识别单词（不在词典中）", rightPanel);
    QVBoxLayout *unknownLayout = new QVBoxLayout(unknownGroup);
    unknownLayout->setContentsMargins(4, 4, 4, 4);
    m_unknownTable = new QTableWidget(0, 1, unknownGroup);
    m_unknownTable->setHorizontalHeaderLabels({"未识别单词"});
    m_unknownTable->horizontalHeader()->setStretchLastSection(true);
    m_unknownTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_unknownTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_unknownTable->setAlternatingRowColors(true);
    m_unknownTable->verticalHeader()->setVisible(false);
    m_unknownTable->verticalHeader()->setDefaultSectionSize(28);
    unknownLayout->addWidget(m_unknownTable);
    rightLayout->addWidget(unknownGroup);

    QGroupBox *newGroup = new QGroupBox("📖 生词（已从词典获取释义）", rightPanel);
    QVBoxLayout *newLayout = new QVBoxLayout(newGroup);
    newLayout->setContentsMargins(4, 4, 4, 4);
    m_newWordsTable = new QTableWidget(0, 3, newGroup);
    m_newWordsTable->setHorizontalHeaderLabels({"单词", "音标", "释义"});
    m_newWordsTable->horizontalHeader()->setStretchLastSection(true);
    m_newWordsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_newWordsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_newWordsTable->setAlternatingRowColors(true);
    m_newWordsTable->verticalHeader()->setVisible(false);
    m_newWordsTable->verticalHeader()->setDefaultSectionSize(28);
    newLayout->addWidget(m_newWordsTable);
    rightLayout->addWidget(newGroup);

    QGroupBox *knownGroup = new QGroupBox("✅ 已在熟词库", rightPanel);
    QVBoxLayout *knownLayout = new QVBoxLayout(knownGroup);
    knownLayout->setContentsMargins(4, 4, 4, 4);
    m_knownTable = new QTableWidget(0, 1, knownGroup);
    m_knownTable->setHorizontalHeaderLabels({"已知单词"});
    m_knownTable->horizontalHeader()->setStretchLastSection(true);
    m_knownTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_knownTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_knownTable->setAlternatingRowColors(true);
    m_knownTable->verticalHeader()->setVisible(false);
    m_knownTable->verticalHeader()->setDefaultSectionSize(28);
    knownLayout->addWidget(m_knownTable);
    rightLayout->addWidget(knownGroup);

    QHBoxLayout *rightBtnLayout = new QHBoxLayout();
    rightBtnLayout->setSpacing(6);

    QPushButton *addBtn = new QPushButton("➕ 录入熟词库", this);
    addBtn->setStyleSheet("background-color: #28a745; font-size: 12px; padding: 6px;");
    connect(addBtn, &QPushButton::clicked, this, &DebugWindow::onAddToKnown);

    QPushButton *removeBtn = new QPushButton("➖ 移出熟词库", this);
    removeBtn->setStyleSheet("background-color: #dc3545; font-size: 12px; padding: 6px;");
    connect(removeBtn, &QPushButton::clicked, this, &DebugWindow::onRemoveFromKnown);

    QPushButton *exportCsvBtn = new QPushButton("📄 CSV", this);
    exportCsvBtn->setStyleSheet("background-color: #6c757d; font-size: 12px; padding: 6px;");
    connect(exportCsvBtn, &QPushButton::clicked, this, &DebugWindow::onExportCsv);

    QPushButton *exportPdfBtn = new QPushButton("📑 PDF", this);
    exportPdfBtn->setStyleSheet("background-color: #fd7e14; font-size: 12px; padding: 6px;");
    connect(exportPdfBtn, &QPushButton::clicked, this, &DebugWindow::onExportPdf);

    QPushButton *exportDocBtn = new QPushButton("📝 DOC", this);
    exportDocBtn->setStyleSheet("background-color: #17a2b8; font-size: 12px; padding: 6px;");
    connect(exportDocBtn, &QPushButton::clicked, this, &DebugWindow::onExportDoc);

    QPushButton *dbManageBtn = new QPushButton("🗄️ 数据库", this);
    dbManageBtn->setStyleSheet("background-color: #17a2b8; font-size: 12px; padding: 6px;");
    connect(dbManageBtn, &QPushButton::clicked, this, &DebugWindow::onOpenDatabaseManager);

    rightBtnLayout->addWidget(addBtn);
    rightBtnLayout->addWidget(removeBtn);
    rightBtnLayout->addWidget(exportCsvBtn);
    rightBtnLayout->addWidget(exportPdfBtn);
    rightBtnLayout->addWidget(exportDocBtn);
    rightBtnLayout->addWidget(dbManageBtn);
    rightLayout->addLayout(rightBtnLayout);

    m_statsLabel = new QLabel("📊 字符数: 0 | 单词数: 0 | 行数: 0", rightPanel);
    m_statsLabel->setAlignment(Qt::AlignRight);
    m_statsLabel->setStyleSheet("color: #4a5568; font-weight: bold; padding: 4px; font-size: 11px; background-color: rgba(0,0,0,0.02); border-radius: 4px;");
    rightLayout->addWidget(m_statsLabel);

    mainLayout->addWidget(leftPanel, 4);
    mainLayout->addWidget(rightPanel, 6);
}

void DebugWindow::appendText(const QString &text)
{
    if (text.isEmpty()) return;
    if (!m_textEdit->toPlainText().isEmpty() && !m_textEdit->toPlainText().endsWith('\n'))
        m_textEdit->append("");
    m_textEdit->append(text);
}

void DebugWindow::updateStats()
{
    QString text = m_textEdit->toPlainText();
    int charCount = 0;
    for (QChar ch : text)
        if (!ch.isSpace()) charCount++;
    QStringList words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    int wordCount = words.size();
    int lineCount = text.isEmpty() ? 0 : text.split('\n').size();
    m_statsLabel->setText(QString("📊 字符数: %1 | 单词数: %2 | 行数: %3")
                              .arg(charCount).arg(wordCount).arg(lineCount));
}

void DebugWindow::setUnknownWordsTable(const QVector<QString> &words)
{
    m_unknownTable->setRowCount(words.size());
    for (int i = 0; i < words.size(); ++i) {
        QTableWidgetItem *item = new QTableWidgetItem(words[i]);
        item->setForeground(QBrush(QColor("#dc3545")));
        m_unknownTable->setItem(i, 0, item);
    }
    m_unknownTable->resizeRowsToContents();
    m_unknownTable->horizontalHeader()->setStretchLastSection(true);
}

void DebugWindow::setNewWordsTable(const QVector<WordEntry> &words)
{
    m_newWordsTable->setRowCount(words.size());
    for (int i = 0; i < words.size(); ++i) {
        QTableWidgetItem *wordItem = new QTableWidgetItem(words[i].word);
        wordItem->setForeground(QBrush(QColor("#dc3545")));
        QTableWidgetItem *phoneticItem = new QTableWidgetItem(words[i].phonetic);
        phoneticItem->setFont(QFont("Arial", 10));
        QTableWidgetItem *transItem = new QTableWidgetItem(words[i].translation);
        m_newWordsTable->setItem(i, 0, wordItem);
        m_newWordsTable->setItem(i, 1, phoneticItem);
        m_newWordsTable->setItem(i, 2, transItem);
    }
    m_newWordsTable->resizeColumnsToContents();
    m_newWordsTable->resizeRowsToContents();
}

void DebugWindow::setKnownWordsTable(const QVector<QString> &words)
{
    m_knownTable->setRowCount(words.size());
    for (int i = 0; i < words.size(); ++i) {
        QTableWidgetItem *item = new QTableWidgetItem(words[i]);
        item->setForeground(QBrush(QColor("#28a745")));
        m_knownTable->setItem(i, 0, item);
    }
    m_knownTable->resizeRowsToContents();
}

void DebugWindow::refreshResultTables(const std::unordered_set<QString> &extractedWords)
{
    m_unknownWords.clear();
    m_newWords.clear();
    m_knownWords.clear();

    QSet<QString> knownSet;
    QList<WordEntry> allKnown = m_dbManager->getAllWords();
    for (const WordEntry &w : allKnown)
        knownSet.insert(w.word);

    int knownCount = 0, newCount = 0, unknownCount = 0;

    for (const QString &w : extractedWords) {
        if (knownSet.contains(w)) {
            m_knownWords.append(w);
            knownCount++;
            continue;
        }
        WordEntry dictEntry = m_lookupService->lookupWord(w);
        if (!dictEntry.word.isEmpty()) {
            m_newWords.append(dictEntry);
            newCount++;
        } else {
            m_unknownWords.append(w);
            unknownCount++;
        }
    }

    setUnknownWordsTable(m_unknownWords);
    setNewWordsTable(m_newWords);
    setKnownWordsTable(m_knownWords);

    QString summary = QString("分析完成 - 熟词: %1, 生词: %2, 未识别: %3").arg(knownCount).arg(newCount).arg(unknownCount);
    m_statusLabel->setText("✅ " + summary);
    log(summary);
}

void DebugWindow::onExtractWords()
{
    QString input = m_textEdit->toPlainText();
    if (input.isEmpty()) {
        QMessageBox::information(this, "提示", "没有可提取的文本");
        return;
    }

    log("开始提取单词，文本长度: " + QString::number(input.length()));
    WordExtractor extractor(input, m_dbManager);
    auto wordsSet = extractor.getWords();
    if (wordsSet.empty()) {
        log("未发现任何英语单词");
        QMessageBox::information(this, "提取结果", "未发现任何英语单词");
        return;
    }
    log(QString("提取到 %1 个唯一单词").arg(wordsSet.size()));
    refreshResultTables(wordsSet);
}

void DebugWindow::onImportFile()
{
    QString filter = "所有支持的文件 (*.txt *.md *.html *.htm *.pdf *.docx *.pptx);;"
                     "文本文件 (*.txt);;PDF (*.pdf);;Word 文档 (*.docx);;PowerPoint (*.pptx)";
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", QString(), filter);
    if (filePath.isEmpty()) return;

    log("导入文件: " + filePath);
    m_statusLabel->setText("⏳ 正在导入文件...");
    QApplication::processEvents();

    ImportResult result = FileImporter::importFile(filePath);
    if (!result.text.isEmpty()) {
        appendText(result.text);
        log("文件文本导入成功，长度: " + QString::number(result.text.length()));
        m_statusLabel->setText("✅ 文件文本导入成功");
    }
    if (!result.imagePaths.isEmpty()) {
        log(QString("从文件中提取到 %1 张图片，开始 OCR").arg(result.imagePaths.size()));
        m_statusLabel->setText(QString("⏳ 正在识别 %1 张图片...").arg(result.imagePaths.size()));
        m_ocrManager->startRecognition(result.imagePaths, "\n---\n");
    }
    if (result.text.isEmpty() && result.imagePaths.isEmpty()) {
        QMessageBox::warning(this, "导入失败", "无法读取文件内容");
        m_statusLabel->setText("❌ 导入失败");
        log("导入失败: 无法读取文件内容");
    }
    updateStats();
}

void DebugWindow::onOcrImages()
{
    QStringList imagePaths = QFileDialog::getOpenFileNames(
        this, "选择要识别的图片（可多选）", QString(),
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.tiff *.gif);;所有文件 (*.*)");
    if (imagePaths.isEmpty()) return;

    log(QString("手动选择 %1 张图片，开始 OCR").arg(imagePaths.size()));
    m_statusLabel->setText(QString("⏳ 正在识别 %1 张图片...").arg(imagePaths.size()));
    m_ocrManager->startRecognition(imagePaths, "\n---\n");
}

void DebugWindow::onClearText()
{
    m_textEdit->clear();
    m_statusLabel->setText("✨ 文本已清空");
    log("文本编辑区已清空");
    updateStats();
}

void DebugWindow::onCopyText()
{
    QString text = m_textEdit->toPlainText();
    if (text.isEmpty()) {
        m_statusLabel->setText("⚠️ 没有文本可复制");
        return;
    }
    QApplication::clipboard()->setText(text);
    m_statusLabel->setText("✅ 已复制到剪贴板");
    log("复制文本到剪贴板，长度: " + QString::number(text.length()));
}

void DebugWindow::onHelp()
{
    QMessageBox::about(this, "帮助",
                       "联系 Bcaid\nQQ:75011847\nWX:YBcaid\n\n"
                       "功能说明：\n"
                       "- 提取单词：从左侧文本提取英语单词，并自动分类到右侧表格\n"
                       "- 文件导入：支持 txt/pdf/docx/pptx 等，自动提取文本和图片 OCR\n"
                       "- 图片 OCR：手动选择图片识别文字\n"
                       "- 表格操作：选中生词或熟词，可录入/移出熟词库，导出生词表\n"
                       "调试输出：窗口左侧下方显示运行日志，便于排查问题。");
}

void DebugWindow::onAbout()
{
    QMessageBox::about(this, "关于",
                       "智能文本提取工具 全功能调试版\n"
                       "基于 Qt 6.8.3\n"
                       "后端：common, core, modules\n"
                       "前端：独立调试窗口，整合文本提取、结果分类、熟词库管理、导出\n"
                       "紧凑 debug 风格，带实时日志输出。");
}

void DebugWindow::onOcrFinished(const QString &mergedText)
{
    if (!mergedText.isEmpty()) {
        appendText("【图片OCR识别结果】");
        appendText(mergedText);
        m_statusLabel->setText("✅ OCR 识别完成");
        log(QString("OCR 识别完成，识别文本长度: %1 字符").arg(mergedText.length()));
        updateStats();
    } else {
        m_statusLabel->setText("⚠️ OCR 未识别到任何文本");
        log("OCR 未识别到任何文本");
    }
}

void DebugWindow::onTextChanged()
{
    updateStats();
}

void DebugWindow::onAddToKnown()
{
    QModelIndexList selected = m_newWordsTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "提示", "请先在生词表格中选中要录入的单词");
        return;
    }

    QList<WordEntry> toAdd;
    for (const QModelIndex &idx : selected) {
        int row = idx.row();
        if (row >= 0 && row < m_newWords.size()) {
            toAdd.append(m_newWords[row]);
        }
    }
    if (toAdd.isEmpty()) return;

    int inserted = m_dbManager->addWords(toAdd);
    if (inserted > 0) {
        for (int i = selected.size()-1; i >= 0; --i) {
            int row = selected[i].row();
            if (row >= 0 && row < m_newWords.size()) {
                m_knownWords.append(m_newWords[row].word);
                m_newWords.remove(row);
            }
        }
        setNewWordsTable(m_newWords);
        setKnownWordsTable(m_knownWords);
        QString msg = QString("成功录入 %1 个单词到熟词库").arg(inserted);
        m_statusLabel->setText("✅ " + msg);
        log(msg);
    } else {
        QMessageBox::warning(this, "失败", "录入失败，可能单词已存在或数据库错误");
        log("录入失败: 可能单词已存在或数据库错误");
    }
}

void DebugWindow::onRemoveFromKnown()
{
    QModelIndexList selected = m_knownTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "提示", "请先在熟词表格中选中要移出的单词");
        return;
    }

    QStringList toRemove;
    for (const QModelIndex &idx : selected) {
        int row = idx.row();
        if (row >= 0 && row < m_knownWords.size()) {
            toRemove << m_knownWords[row];
        }
    }
    if (toRemove.isEmpty()) return;

    int deleted = m_dbManager->removeWords(toRemove);
    if (deleted > 0) {
        for (int i = selected.size()-1; i >= 0; --i) {
            int row = selected[i].row();
            if (row >= 0 && row < m_knownWords.size()) {
                QString word = m_knownWords[row];
                m_knownWords.remove(row);
                m_unknownWords.append(word);
            }
        }
        setKnownWordsTable(m_knownWords);
        setUnknownWordsTable(m_unknownWords);
        QString msg = QString("成功从熟词库移出 %1 个单词").arg(deleted);
        m_statusLabel->setText("✅ " + msg);
        log(msg);
    } else {
        QMessageBox::warning(this, "失败", "移出失败");
        log("移出熟词库失败");
    }
}

void DebugWindow::onExportCsv()
{
    if (m_newWords.isEmpty()) {
        QMessageBox::information(this, "提示", "没有生词可导出");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this, "导出生词表 CSV", QDir::homePath() + "/生词表.csv", "CSV文件 (*.csv)");
    if (fileName.isEmpty()) return;
    bool ok = ExportManager::exportToCsv(m_newWords.toList(), fileName);
    if (ok) {
        QMessageBox::information(this, "导出成功", QString("已导出到 %1").arg(fileName));
        log("导出 CSV 成功: " + fileName);
    } else {
        QMessageBox::warning(this, "导出失败", "无法创建文件");
        log("导出 CSV 失败");
    }
}

void DebugWindow::onExportPdf()
{
    if (m_newWords.isEmpty()) {
        QMessageBox::information(this, "提示", "没有生词可导出");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this, "导出生词表 PDF", QDir::homePath() + "/生词表.pdf", "PDF文件 (*.pdf)");
    if (fileName.isEmpty()) return;
    bool ok = ExportManager::exportToPdf(m_newWords.toList(), fileName);
    if (ok) {
        QMessageBox::information(this, "导出成功", QString("已导出到 %1").arg(fileName));
        log("导出 PDF 成功: " + fileName);
    } else {
        QMessageBox::warning(this, "导出失败", "PDF 生成失败");
        log("导出 PDF 失败");
    }
}

void DebugWindow::onExportDoc()
{
    if (m_newWords.isEmpty()) {
        QMessageBox::information(this, "提示", "没有生词可导出");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this, "导出生词表 Word", QDir::homePath() + "/生词表.doc", "Word文档 (*.doc)");
    if (fileName.isEmpty()) return;
    bool ok = ExportManager::exportToDoc(m_newWords.toList(), fileName);
    if (ok) {
        QMessageBox::information(this, "导出成功", QString("已导出到 %1").arg(fileName));
        log("导出 DOC 成功: " + fileName);
    } else {
        QMessageBox::warning(this, "导出失败", "DOC 生成失败");
        log("导出 DOC 失败");
    }
}

void DebugWindow::onOpenDatabaseManager()
{
    log("打开数据库管理窗口");
    DatabaseManagementWindow *win = new DatabaseManagementWindow(m_dbManager->database(), nullptr);
    win->setAttribute(Qt::WA_DeleteOnClose);
    win->show();
}

void DebugWindow::onCommandEntered()
{
    QString cmd = m_commandInput->text().trimmed();
    if (cmd.isEmpty()) return;

    log("执行命令: " + cmd);
    m_commandInput->clear();

    if (cmd == "help") {
        QString helpText =
            "可用命令列表：\n"
            "  help                        - 显示本帮助信息\n"
            "  checkDir                    - 显示项目目录树\n"
            "  exportCode [路径]            - 导出项目源代码到 combined_for_ai.txt\n"
            "  exportQml [路径]            - 导出项目qml代码到 combined_qml.txt\n"
            "  open qml <文件路径> 或 oq <路径> - 在新窗口中运行指定的 QML 文件\n"
            "  log clear                   - 清空 data/logs/ 目录下的所有 .log 日志文件\n";
        log(helpText);
        m_statusLabel->setText("✅ 已输出帮助信息到调试输出区");
        return;
    }

    if (cmd.startsWith("open qml ") || cmd.startsWith("oq ")) {
        QString qmlPath;
        if (cmd.startsWith("open qml ")) {
            qmlPath = cmd.mid(9).trimmed();
        } else if (cmd.startsWith("oq ")) {
            qmlPath = cmd.mid(3).trimmed();
        }
        if (qmlPath.isEmpty()) {
            log("错误：未提供 QML 文件路径，用法: open qml <路径> 或 oq <路径>");
            m_statusLabel->setText("❌ 缺少 QML 文件路径");
        } else if (qmlPath == "main") {
            openQmlWindow(PROJECTROOT + "/ui/main/main.qml");
        } else {
            openQmlWindow(qmlPath);
        }
        return;
    }

    if (cmd == "log clear" || cmd == "logclear" || cmd == "clearlog" || cmd == "clog") {
        clearLogFiles();
        return;
    }

    if (cmd.startsWith("exportCode", Qt::CaseInsensitive) || cmd.startsWith("excode", Qt::CaseInsensitive)) {
        QStringList parts = cmd.split(' ', Qt::SkipEmptyParts);
        QString targetPath;
        if (parts.size() >= 2) {
            targetPath = parts[1];
        } else {
            targetPath = PROJECTROOT;
        }

        log("开始导出项目代码，根目录: " + targetPath);
        DebugTool tool;
        QString combined = tool.combineAllFilesForAI(targetPath);

        QDir().mkpath(PROJECTROOT + "/data");
        QString outputFileName = PROJECTROOT + "/data/combined_for_ai.txt";
        QFile file(outputFileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << combined;
            file.close();
            qint64 fileSize = QFileInfo(outputFileName).size();
            log(QString("导出成功！文件已保存至: %1 (%2 字节)").arg(outputFileName).arg(fileSize));
            m_statusLabel->setText(QString("✅ 代码导出成功: %1").arg(outputFileName));
        } else {
            log("导出失败：无法写入文件 " + outputFileName);
            m_statusLabel->setText("❌ 导出失败，无法写入文件");
        }
    }
    else if (cmd.startsWith("exportQml", Qt::CaseInsensitive) || cmd.startsWith("exqml", Qt::CaseInsensitive)) {
        exportQmlFiles();
    }
    else if (cmd == "checkDir" || cmd == "chkdir") {
        log("正在生成项目目录树...");
        DebugTool tool;
        QString tree = tool.generateDirectoryTree(PROJECTROOT);
        QStringList lines = tree.split('\n');
        log("----- 项目目录树 (" + PROJECTROOT + ") -----");
        for (const QString &line : lines) {
            if (m_debugOutput) {
                m_debugOutput->appendPlainText(line);
            }
        }
        log("----- 目录树输出结束 -----");
    } else {
        log("未知命令: " + cmd + "。支持的命令: checkDir, exportCode [可选路径]");
    }
}

void DebugWindow::openQmlWindow(const QString &qmlPath)
{
    QFileInfo fileInfo(qmlPath);
    if (!fileInfo.exists()) {
        log("QML 文件不存在: " + qmlPath);
        return;
    }

    QQmlApplicationEngine *engine = new QQmlApplicationEngine();
    engine->addImportPath(fileInfo.absolutePath());
    QObject::connect(engine, &QQmlApplicationEngine::objectCreationFailed,
                     [engine, qmlPath]() {
                         qDebug() << "QML 引擎创建失败:" << qmlPath;
                         engine->deleteLater();
                     });
    engine->load(QUrl::fromLocalFile(qmlPath));

    QObject::connect(engine, &QQmlApplicationEngine::destroyed, engine, &QObject::deleteLater);
    log("已打开 QML 窗口: " + qmlPath);
}

void DebugWindow::clearLogFiles()
{
    Logger::instance().clearTargets();
    auto consoleTarget = QSharedPointer<ConsoleTarget>::create();
    Logger::instance().addTargetForAllCategories(consoleTarget);

    QString logsDirPath = PROJECTROOT + "/data/logs";
    QDir logsDir(logsDirPath);
    if (!logsDir.exists()) {
        log("日志目录不存在: " + logsDirPath);
        return;
    }

    QStringList filters{"*.log"};
    QStringList logFiles = logsDir.entryList(filters, QDir::Files);
    int removedCount = 0, failCount = 0;
    for (const QString &fileName : logFiles) {
        QString fullPath = logsDir.absoluteFilePath(fileName);
        if (QFile::remove(fullPath))
            removedCount++;
        else
            failCount++;
    }

    QDir().mkpath(logsDirPath);
    const LogCategory categories[] = {
        LogCategory::General, LogCategory::Database, LogCategory::Extract,
        LogCategory::Ocr, LogCategory::Import, LogCategory::Export,
        LogCategory::UI, LogCategory::Config, LogCategory::Network, LogCategory::Update
    };
    const char* fileNames[] = {
        "general.log", "database.log", "extract.log", "ocr.log", "import.log",
        "export.log", "ui.log", "config.log", "network.log", "update.log"
    };
    for (int i = 0; i < 10; ++i) {
        QString filePath = logsDirPath + "/" + fileNames[i];
        auto fileTarget = QSharedPointer<FileTarget>::create(filePath);
        Logger::instance().addTarget(fileTarget, categories[i]);
    }
    log(QString("日志清理完成: 成功删除 %1 个，失败 %2 个").arg(removedCount).arg(failCount));
}

void DebugWindow::exportQmlFiles()
{
    const QString projectRoot = PROJECTROOT;
    QDir().mkpath(PROJECTROOT + "/data");
    QString outputFileName = PROJECTROOT + "/data/combined_qml.txt";

    QDir rootDir(projectRoot);
    if (!rootDir.exists()) {
        log("错误：项目根目录不存在 - " + projectRoot);
        return;
    }

    QStringList qmlFiles;
    QDirIterator it(projectRoot, QStringList() << "*.qml", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        if (filePath.contains("/build/") || filePath.contains("/.git/") ||
            filePath.contains("/Debug/") || filePath.contains("/Release/"))
            continue;
        qmlFiles << filePath;
    }

    if (qmlFiles.isEmpty()) {
        log("未找到任何 .qml 文件");
        return;
    }

    QFile outFile(outputFileName);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        log("无法创建输出文件：" + outputFileName);
        return;
    }

    QTextStream out(&outFile);
    out.setEncoding(QStringConverter::Utf8);
    out << "// ============================================================\n";
    out << "// Combined QML files from project: " << projectRoot << "\n";
    out << "// Total QML files: " << qmlFiles.size() << "\n";
    out << "// ============================================================\n\n";

    for (const QString &filePath : qmlFiles) {
        QFile qmlFile(filePath);
        if (!qmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            out << "// [ERROR] Cannot open: " << filePath << "\n\n";
            continue;
        }
        QTextStream in(&qmlFile);
        in.setEncoding(QStringConverter::Utf8);
        QString content = in.readAll();
        qmlFile.close();

        QString relativePath = QDir(projectRoot).relativeFilePath(filePath);
        out << "// ============================================================\n";
        out << "// File: " << relativePath << "\n";
        out << "// ============================================================\n";
        out << content;
        if (!content.endsWith('\n'))
            out << "\n";
        out << "// ============================================================\n";
        out << "// End of file: " << relativePath << "\n";
        out << "// ============================================================\n\n";
    }

    outFile.close();
    qint64 fileSize = QFileInfo(outputFileName).size();
    log(QString("✅ QML 导出成功！文件保存至: %1 (%2 字节，共 %3 个 QML 文件)")
            .arg(outputFileName).arg(fileSize).arg(qmlFiles.size()));
    m_statusLabel->setText(QString("✅ QML 导出成功: %1").arg(outputFileName));
}