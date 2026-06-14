#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#include <QQuickView>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QStatusBar>
#include <QTemporaryDir>
#include <QVector>
#include <unordered_set>
#include <QSet>
#include "DatabaseManagementWindow.h"
#include "../core/dictionary/LookupService.h"

class WordDatabaseManager;
class OcrManager;
struct WordEntry;

/**
 * @brief 完整调试窗口，整合文本提取、结果分类、熟词库管理和导出功能。
 */
class DebugWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DebugWindow(QWidget *parent = nullptr);
    ~DebugWindow();

    /// 输出消息到调试日志区域（带时间戳）
    void log(const QString &msg);

private slots:
    void onExtractWords();
    void onImportFile();
    void onOcrImages();
    void onClearText();
    void onCopyText();
    void onHelp();
    void onAbout();

    void onAddToKnown();
    void onRemoveFromKnown();
    void onExportCsv();
    void onExportPdf();
    void onExportDoc();

    void onOpenDatabaseManager();

    void onOcrFinished(const QString &mergedText);
    void onTextChanged();

    void onCommandEntered();

private:
    void setupUI();
    void updateStats();
    void appendText(const QString &text);
    void refreshResultTables(const std::unordered_set<QString> &extractedWords);

    void openQmlWindow(const QString &qmlPath);
    void clearLogFiles();
    void exportQmlFiles();

    void setUnknownWordsTable(const QVector<QString> &words);
    void setNewWordsTable(const QVector<WordEntry> &words);
    void setKnownWordsTable(const QVector<QString> &words);

    QTextEdit       *m_textEdit;
    QPlainTextEdit  *m_debugOutput;
    QLineEdit       *m_commandInput;
    QTableWidget    *m_unknownTable;
    QTableWidget    *m_newWordsTable;
    QTableWidget    *m_knownTable;
    QLabel          *m_statsLabel;
    QLabel          *m_statusLabel;

    WordDatabaseManager *m_dbManager;
    OcrManager          *m_ocrManager;

    QVector<QString>     m_unknownWords;
    QVector<WordEntry>   m_newWords;
    QVector<QString>     m_knownWords;

    QTemporaryDir         m_tempDir;
    LookupService        *m_lookupService;
};

#endif // DEBUGWINDOW_H