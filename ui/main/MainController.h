#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QVariantList>
#include <QFutureWatcher>
#include <QPointer>
#include <QString>
#include <unordered_set>
#include "../common/WordEntry.h"

class WordDatabaseManager;
class LookupService;
class OcrManager;

class MainController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(int knownWordCount READ knownWordCount NOTIFY wordStatsChanged)
    Q_PROPERTY(int newWordCount READ newWordCount NOTIFY wordStatsChanged)
    Q_PROPERTY(int unknownWordCount READ unknownWordCount NOTIFY wordStatsChanged)
    Q_PROPERTY(bool isProcessing READ isProcessing NOTIFY processingChanged)

public:
    explicit MainController(QObject *parent = nullptr);
    ~MainController();

    QString statusMessage() const { return m_statusMessage; }
    int knownWordCount() const { return m_knownWords.size(); }
    int newWordCount() const { return m_newWords.size(); }
    int unknownWordCount() const { return m_unknownWords.size(); }
    bool isProcessing() const { return m_isProcessing; }

    Q_INVOKABLE void extractWords(const QString &text);
    Q_INVOKABLE void importFile(const QString &filePath);
    Q_INVOKABLE void selectAndOcrImages();
    Q_INVOKABLE void openDatabaseManager();
    Q_INVOKABLE void clearText();
    Q_INVOKABLE void copyToClipboard(const QString &text);
    Q_INVOKABLE void showHelp();
    Q_INVOKABLE void showAbout();
    Q_INVOKABLE void addToKnown(const QVariantList &selectedIndexes);
    Q_INVOKABLE void removeFromKnown(const QVariantList &selectedIndexes);
    Q_INVOKABLE void exportCsv();
    Q_INVOKABLE void exportPdf();
    Q_INVOKABLE void exportDoc();
    Q_INVOKABLE QVariantList knownWordsList() const;
    Q_INVOKABLE QVariantList newWordsList() const;
    Q_INVOKABLE QVariantList unknownWordsList() const;

signals:
    void statusMessageChanged();
    void wordStatsChanged();
    void processingChanged();
    void extractionFinished();
    void ocrFinished(const QString &mergedText);
    void fileImportFinished(const QString &text, const QStringList &imagePaths);
    void operationFailed(const QString &errorMessage);

private slots:
    void onExtractWordsFinished();
    void onOcrFinished(const QString &mergedText);

private:
    void initBackend();
    void setStatusMessage(const QString &msg);
    void setProcessing(bool processing);
    void classifyWords(const std::unordered_set<QString> &extractedWords);
    void updateWordStats();

    QPointer<WordDatabaseManager> m_dbManager;
    QPointer<LookupService>      m_lookupService;
    QPointer<OcrManager>         m_ocrManager;

    QStringList m_knownWords;          // 熟词（仅单词）
    QList<struct WordEntry> m_newWords; // 生词（完整 WordEntry）
    QStringList m_unknownWords;        // 未识别

    QString m_statusMessage;
    bool    m_isProcessing = false;

    QFutureWatcher<void> m_extractWatcher;
};

#endif // MAINCONTROLLER_H