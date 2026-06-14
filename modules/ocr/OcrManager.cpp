// OcrManager.cpp
#include "OcrManager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

OcrTask::OcrTask(const QString &imagePath,
                 const QString &ocrExecutablePath,
                 const QString &tessdataDirectory,
                 const QString &language,
                 OcrManager *manager,
                 int index)
    : QRunnable(),
    m_imagePath(imagePath),
    m_ocrExecutablePath(ocrExecutablePath),
    m_tessdataDirectory(tessdataDirectory),
    m_language(language),
    m_manager(manager),
    m_index(index)
{
    setAutoDelete(true);
}

void OcrTask::run()
{
    QProcess process;
    QStringList args;
    args << m_imagePath;
    args << "-l" << m_language;
    if (!m_tessdataDirectory.isEmpty())
    {
        args << "-t" << m_tessdataDirectory;
    }

    bool success = false;
    QString recognizedText;

    process.start(m_ocrExecutablePath, args);
    if (process.waitForStarted(5000))
    {
        if (process.waitForFinished(30000))
        {
            if (process.exitCode() == 0)
            {
                QByteArray output = process.readAllStandardOutput();
                recognizedText = QString::fromUtf8(output).trimmed();
                success = true;
            }
            else
            {
                qDebug() << "OCR exited with code" << process.exitCode() << "for" << m_imagePath;
            }
        }
        else
        {
            qDebug() << "OCR timeout for" << m_imagePath;
            process.kill();
        }
    }
    else
    {
        qDebug() << "Failed to start OCR for" << m_imagePath;
    }

    m_manager->onTaskFinished(m_index, success, recognizedText);
}

OcrManager::OcrManager(QObject *parent)
    : QObject(parent)
    , m_ocrExecutablePath(QCoreApplication::applicationDirPath() + "/EUWEOCRT.exe")
    , m_tessdataDirectory(QCoreApplication::applicationDirPath() + "/tessdata")
    , m_language("eng")
    , m_maxThreads(QThread::idealThreadCount())
    , m_pendingCount(0)
{
    m_ocrThreadPool.setMaxThreadCount(m_maxThreads);
}

OcrManager::~OcrManager()
{
    m_ocrThreadPool.waitForDone();
}

void OcrManager::setOcrProgramPath(const QString &path)
{
    m_ocrExecutablePath = path;
}

void OcrManager::setTessdataPath(const QString &path)
{
    m_tessdataDirectory = path;
}

void OcrManager::setLanguage(const QString &lang)
{
    m_language = lang;
}

void OcrManager::setMaxThreads(int maxThreads)
{
    if (maxThreads > 0)
    {
        m_maxThreads = maxThreads;
        m_ocrThreadPool.setMaxThreadCount(m_maxThreads);
    }
}

QString OcrManager::recognizeImages(const QStringList &imagePaths, const QString &separator)
{
    if (imagePaths.isEmpty())
    {
        return QString();
    }

    {
        QMutexLocker locker(&m_mutex);
        m_pendingCount = imagePaths.size();
        m_results.clear();
        m_results.resize(imagePaths.size());
        m_successFlags.clear();
        m_successFlags.resize(imagePaths.size(), false);
    }

    for (int i = 0; i < imagePaths.size(); ++i)
    {
        OcrTask *task = new OcrTask(imagePaths[i],
                                    m_ocrExecutablePath,
                                    m_tessdataDirectory,
                                    m_language,
                                    this,
                                    i);
        m_ocrThreadPool.start(task);
    }

    QMutexLocker locker(&m_mutex);
    while (m_pendingCount > 0)
    {
        m_waitCondition.wait(&m_mutex);
    }

    QStringList validTexts;
    for (int i = 0; i < m_results.size(); ++i)
    {
        if (m_successFlags[i] && !m_results[i].isEmpty())
        {
            validTexts << m_results[i];
        }
    }
    return validTexts.join(separator);
}

void OcrManager::startRecognition(const QStringList &imagePaths, const QString &separator)
{
    QtConcurrent::run(
        [this, imagePaths, separator]()
        {
            QString result = recognizeImages(imagePaths, separator);
            emit finished(result);
        });
}

void OcrManager::onTaskFinished(int index, bool success, const QString &text)
{
    QMutexLocker locker(&m_mutex);
    m_results[index] = text;
    m_successFlags[index] = success;
    m_pendingCount--;
    m_waitCondition.wakeOne();
}