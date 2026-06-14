// OcrManager.h
#ifndef OCRMANAGER_H
#define OCRMANAGER_H

#include <QAtomicInt>
#include <QCoreApplication>
#include <QMutex>
#include <QObject>
#include <QProcess>
#include <QRunnable>
#include <QStringList>
#include <QThreadPool>
#include <QWaitCondition>
#include <QtConcurrent/QtConcurrent>

class OcrManager;

/**
 * @brief 单张图片 OCR 任务，在独立线程中调用外部 OCR 可执行程序。
 */
class OcrTask : public QRunnable {
public:
    OcrTask(const QString &imagePath,
            const QString &ocrExecutablePath,
            const QString &tessdataDirectory,
            const QString &language,
            OcrManager *manager,
            int index);

    void run() override;

private:
    QString m_imagePath;
    QString m_ocrExecutablePath;
    QString m_tessdataDirectory;
    QString m_language;
    OcrManager *m_manager;
    int m_index;
};

/**
 * @brief 多线程 OCR 管理器。管理 OCR 任务池，支持同步/异步批量识别，
 *        合并所有识别结果。
 */
class OcrManager : public QObject
{
    Q_OBJECT

public:
    explicit OcrManager(QObject *parent = nullptr);
    ~OcrManager();

    void setOcrProgramPath(const QString &path);
    void setTessdataPath(const QString &path);
    void setLanguage(const QString &lang);
    void setMaxThreads(int maxThreads);

    /// 同步识别多张图片，阻塞直到全部完成，返回合并后的文本。
    QString recognizeImages(const QStringList &imagePaths,
                            const QString &separator = "\n---\n");

    /// 异步识别多张图片，完成后发送 finished 信号。
    void startRecognition(const QStringList &imagePaths,
                          const QString &separator = "\n---\n");

signals:
    void finished(const QString &mergedText);

private:
    void onTaskFinished(int index, bool success, const QString &text);

    QString m_ocrExecutablePath;
    QString m_tessdataDirectory;
    QString m_language;
    int m_maxThreads;

    QThreadPool m_ocrThreadPool;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;

    int m_pendingCount;
    QList<QString> m_results;
    QList<bool> m_successFlags;

    friend class OcrTask;
};

#endif // OCRMANAGER_H