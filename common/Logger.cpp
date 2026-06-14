// Logger.cpp
#include "Logger.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSaveFile>
#include <QThread>

// ============================================================================
// LogMessage 实现
// ============================================================================
LogMessage::LogMessage(LogLevel level, LogCategory category,
                       const char *file, int line, const char *func)
    : m_level(level)
    , m_category(category)
    , m_file(file)
    , m_line(line)
    , m_func(func)
    , m_buffer()
    , m_debug(&m_buffer)
{
    m_debug.setAutoInsertSpaces(false);
}

LogMessage::~LogMessage()
{
    Logger::instance().write(m_level, m_category, m_buffer, m_file, m_line, m_func);
}

QDebug &LogMessage::stream()
{
    return m_debug;
}

// ============================================================================
// ConsoleTarget 实现
// ============================================================================
void ConsoleTarget::write(const QString &formattedMessage)
{
    qDebug().noquote() << formattedMessage;
}

// ============================================================================
// FileTarget 实现
// ============================================================================
FileTarget::FileTarget(const QString &filePath, qint64 maxSizeBytes, int backupCount)
    : m_filePath(filePath)
    , m_maxSize(maxSizeBytes)
    , m_backupCount(backupCount)
{
    QDir().mkpath(QFileInfo(m_filePath).absolutePath());
    openFile();
}

FileTarget::~FileTarget()
{
    QMutexLocker locker(&m_fileMutex);
    if (m_file.isOpen())
    {
        m_stream.flush();
        m_file.close();
    }
}

void FileTarget::write(const QString &formattedMessage)
{
    QMutexLocker locker(&m_fileMutex);

    if (!m_file.isOpen())
        openFile();

    if (m_file.isOpen())
    {
        m_stream << formattedMessage << Qt::endl;
        m_stream.flush();

        if (m_maxSize > 0 && m_file.size() > m_maxSize)
            rollFile();
    }
}

void FileTarget::openFile()
{
    if (m_file.isOpen())
        m_file.close();

    m_file.setFileName(m_filePath);
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        qWarning() << "Logger: Cannot open log file" << m_filePath << ":" << m_file.errorString();
        return;
    }
    m_stream.setDevice(&m_file);
}

void FileTarget::rollFile()
{
    m_stream.flush();
    m_file.close();

    const QString oldestBackup = m_filePath + "." + QString::number(m_backupCount);
    if (QFile::exists(oldestBackup))
        QFile::remove(oldestBackup);

    for (int i = m_backupCount - 1; i >= 1; --i)
    {
        const QString src = m_filePath + "." + QString::number(i);
        const QString dst = m_filePath + "." + QString::number(i + 1);
        if (QFile::exists(src))
        {
            if (QFile::exists(dst))
                QFile::remove(dst);
            QFile::rename(src, dst);
        }
    }
    QFile::rename(m_filePath, m_filePath + ".1");
    openFile();
}

// ============================================================================
// Logger 单例及核心实现
// ============================================================================
Logger &Logger::instance()
{
    static Logger inst;
    return inst;
}

void Logger::addTarget(const QSharedPointer<LoggerTarget> &target, LogCategory category)
{
    if (!target)
        return;

    QMutexLocker locker(&m_mutex);
    m_targets[category].append(target);
}

void Logger::addTargetForAllCategories(const QSharedPointer<LoggerTarget> &target)
{
    if (!target)
        return;

    QMutexLocker locker(&m_mutex);
    // 遍历所有已存在的分类（未来新增的分类不会自动添加，需调用者保证）
    for (auto it = m_targets.begin(); it != m_targets.end(); ++it)
        it.value().append(target);
    // 同时也为 General 分类添加（确保至少有一个）
    m_targets[LogCategory::General].append(target);
}

void Logger::clearTargets()
{
    QMutexLocker locker(&m_mutex);
    m_targets.clear();
}

void Logger::setGlobalLevel(LogLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_globalLevel = level;
}

LogLevel Logger::globalLevel() const
{
    QMutexLocker locker(&m_mutex);
    return m_globalLevel;
}

void Logger::write(LogLevel level, LogCategory category,
                   const QString &message, const char *file, int line, const char *func)
{
    // 级别过滤已在宏中完成，此处仅做格式化
    const QString formatted = formatMessage(level, category, message, file, line, func);

    QMutexLocker locker(&m_mutex);
    // 获取对应分类的目标列表，如果没有则使用 General 分类的目标（fallback）
    auto it = m_targets.find(category);
    if (it != m_targets.end())
    {
        for (const auto &target : it.value())
            if (target)
                target->write(formatted);
    }
    // 同时也要写入 General 分类的目标（如果想避免重复，可以调整策略，此处独立）
    auto genIt = m_targets.find(LogCategory::General);
    if (genIt != m_targets.end())
    {
        for (const auto &target : genIt.value())
            if (target)
                target->write(formatted);
    }
}

QString Logger::formatMessage(LogLevel level, LogCategory category,
                              const QString &msg, const char *file, int line, const char *func)
{
    static const char *levelStrings[] = {"DEBUG", "INFO", "WARNING", "ERROR", "FATAL"};
    const char *levelStr = levelStrings[static_cast<int>(level)];

    static const QHash<LogCategory, QString> categoryStrings = []{
        QHash<LogCategory, QString> map;
        map[LogCategory::General]   = "GEN";
        map[LogCategory::Database]  = "DB";
        map[LogCategory::Extract]   = "EXTRACT";
        map[LogCategory::Ocr]       = "OCR";
        map[LogCategory::Import]    = "IMPORT";
        map[LogCategory::Export]    = "EXPORT";
        map[LogCategory::UI]        = "UI";
        map[LogCategory::Config]    = "CONFIG";
        map[LogCategory::Network]   = "NET";
        map[LogCategory::Update]    = "UPDATE";
        return map;
    }();
    QString catStr = categoryStrings.value(category, "???");

    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
    const QString threadId = QStringLiteral("0x%1").arg(reinterpret_cast<quintptr>(QThread::currentThreadId()), 0, 16);

    QString result;
    QTextStream ts(&result);
    ts << "[" << timestamp << "] "
       << "[" << levelStr << "] "
       << "[" << catStr << "] "
       << "[" << threadId << "] "
       << file << ":" << line << " - " << func << ": " << msg;
    return result;
}