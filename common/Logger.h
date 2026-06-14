// Logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QTextStream>

/**
 * @brief 日志级别枚举（严重程度递增）
 */
enum class LogLevel
{
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    FATAL = 4
};

/**
 * @brief 日志分类枚举（用于按模块分流）
 */
enum class LogCategory
{
    General,        // 通用/默认
    Database,       // 数据库操作
    Extract,        // 单词提取
    Ocr,            // OCR 识别
    Import,         // 文件导入
    Export,         // 导出生词表
    UI,             // 界面交互
    Config,         // 配置读写
    Network,        // 网络/本地服务器
    Update,         // 更新检查
    // 可根据需要扩展
};

class LoggerTarget;
class Logger;

// 前置声明，用于宏定义
Q_DECLARE_METATYPE(LogCategory)

/**
 * @brief 日志消息构建器（RAII 风格，支持分类）
 */
class LogMessage
{
public:
    LogMessage(LogLevel level, LogCategory category,
               const char *file, int line, const char *func);
    ~LogMessage();

    QDebug &stream();

private:
    LogLevel m_level;
    LogCategory m_category;
    const char *m_file;
    int m_line;
    const char *m_func;
    QString m_buffer;
    QDebug m_debug;

    Q_DISABLE_COPY_MOVE(LogMessage)
};

/**
 * @brief 日志输出目标抽象基类
 */
class LoggerTarget
{
public:
    virtual ~LoggerTarget() = default;
    virtual void write(const QString &formattedMessage) = 0;
};

/**
 * @brief 控制台输出目标（输出到 qDebug）
 */
class ConsoleTarget : public LoggerTarget
{
public:
    void write(const QString &formattedMessage) override;
};

/**
 * @brief 文件输出目标（支持大小滚动）
 */
class FileTarget : public LoggerTarget
{
public:
    explicit FileTarget(const QString &filePath,
                        qint64 maxSizeBytes = 10 * 1024 * 1024,
                        int backupCount = 5);
    ~FileTarget() override;

    void write(const QString &formattedMessage) override;

private:
    void openFile();
    void rollFile();

    QString m_filePath;
    qint64 m_maxSize;
    int m_backupCount;
    QFile m_file;
    QTextStream m_stream;
    QMutex m_fileMutex;
};

/**
 * @brief 全局日志管理器（线程安全单例，支持分类路由）
 */
class Logger
{
public:
    static Logger &instance();

    // 为特定分类添加目标（若 category 为 General，则仅用于 General 分类）
    void addTarget(const QSharedPointer<LoggerTarget> &target,
                   LogCategory category = LogCategory::General);

    // 为目标添加所有分类（包括未来新增的分类，需谨慎使用）
    void addTargetForAllCategories(const QSharedPointer<LoggerTarget> &target);

    // 移除所有目标（慎用，通常用于单元测试重置）
    void clearTargets();

    // 设置全局最低日志级别（所有分类共用）
    void setGlobalLevel(LogLevel level);
    LogLevel globalLevel() const;

    // 写入日志（由 LogMessage 析构时调用）
    void write(LogLevel level, LogCategory category,
               const QString &message, const char *file, int line, const char *func);

private:
    Logger() = default;
    ~Logger() = default;
    Q_DISABLE_COPY_MOVE(Logger)

    // 格式化为单行字符串
    QString formatMessage(LogLevel level, LogCategory category,
                          const QString &msg, const char *file, int line, const char *func);

    // 分类 -> 目标列表映射
    QHash<LogCategory, QList<QSharedPointer<LoggerTarget>>> m_targets;
    LogLevel m_globalLevel = LogLevel::DEBUG;
    mutable QMutex m_mutex;
};

// 带分类的日志宏
#define LOG_DEBUG_CAT(cat) \
if (Logger::instance().globalLevel() > LogLevel::DEBUG) {} \
    else LogMessage(LogLevel::DEBUG, cat, __FILE__, __LINE__, Q_FUNC_INFO).stream()

#define LOG_INFO_CAT(cat) \
    if (Logger::instance().globalLevel() > LogLevel::INFO) {} \
    else LogMessage(LogLevel::INFO, cat, __FILE__, __LINE__, Q_FUNC_INFO).stream()

#define LOG_WARNING_CAT(cat) \
    if (Logger::instance().globalLevel() > LogLevel::WARNING) {} \
    else LogMessage(LogLevel::WARNING, cat, __FILE__, __LINE__, Q_FUNC_INFO).stream()

#define LOG_ERROR_CAT(cat) \
    if (Logger::instance().globalLevel() > LogLevel::ERROR) {} \
    else LogMessage(LogLevel::ERROR, cat, __FILE__, __LINE__, Q_FUNC_INFO).stream()

#define LOG_FATAL_CAT(cat) \
    if (Logger::instance().globalLevel() > LogLevel::FATAL) {} \
    else LogMessage(LogLevel::FATAL, cat, __FILE__, __LINE__, Q_FUNC_INFO).stream()

// 兼容旧代码的默认分类（使用 General）
#define LOG_DEBUG()   LOG_DEBUG_CAT(LogCategory::General)
#define LOG_INFO()    LOG_INFO_CAT(LogCategory::General)
#define LOG_WARNING() LOG_WARNING_CAT(LogCategory::General)
#define LOG_ERROR()   LOG_ERROR_CAT(LogCategory::General)
#define LOG_FATAL()   LOG_FATAL_CAT(LogCategory::General)

#endif // LOGGER_H