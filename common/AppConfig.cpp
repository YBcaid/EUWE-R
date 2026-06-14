// appconfig.cpp
#include "appconfig.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSaveFile>
#include <QStandardPaths>

// 静态成员初始化
QScopedPointer<AppConfig> AppConfig::s_instance;
QMutex AppConfig::s_instanceMutex;

/**
 * @brief 私有构造函数
 *
 * 设置默认配置文件路径（基于 QStandardPaths），不进行文件加载。
 * 路径格式：<AppDataLocation>/config.json
 */
AppConfig::AppConfig()
    : QObject(nullptr)
{
    m_filePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config.json";
}

/**
 * @brief 析构函数
 *
 * 在持有 m_mutex 的情况下，若已初始化则写入配置到磁盘。
 * 确保程序正常退出时不丢失未保存的更改。
 */
AppConfig::~AppConfig()
{
    QMutexLocker locker(&m_mutex);
    if (m_initialized)
    {
        writeToDisk();
    }
}

/**
 * @brief 初始化单例路径（线程安全）
 * @param filePath 自定义文件路径，为空则使用默认路径
 *
 * 若单例尚未创建，则先创建。若已加载过配置（m_initialized == true），
 * 本次调用传入的非空路径将被忽略并输出警告，以免运行时切换配置文件导致混乱。
 */
void AppConfig::initialize(const QString &filePath)
{
    QMutexLocker locker(&s_instanceMutex);
    if (s_instance.isNull())
    {
        s_instance.reset(new AppConfig());
    }
    if (!filePath.isEmpty())
    {
        QMutexLocker cfgLocker(&s_instance->m_mutex);
        if (s_instance->m_initialized)
        {
            qWarning(
                "AppConfig::initialize called after config already loaded. Path change ignored.");
        }
        else
        {
            s_instance->m_filePath = filePath;
        }
    }
}

/**
 * @brief 获取单例实例，并在首次访问时触发惰性加载
 * @return 单例引用
 *
 * 加载过程由 loadInternal() 完成，加载失败将持有空 JSON 对象，不会导致程序崩溃。
 */
AppConfig &AppConfig::instance()
{
    QMutexLocker locker(&s_instanceMutex);
    if (s_instance.isNull())
    {
        s_instance.reset(new AppConfig());
    }
    {
        QMutexLocker cfgLocker(&s_instance->m_mutex);
        if (!s_instance->m_initialized)
        {
            s_instance->loadInternal();
        }
    }
    return *s_instance;
}

/**
 * @brief 内部加载实现（const，通过 mutable 成员修改状态）
 *
 * 流程：
 * 1. 检查是否已初始化（避免重复加载）。
 * 2. 尝试打开文件，若不存在则创建目录并保持空对象。
 * 3. 打开失败或解析失败则初始化空对象，保证 m_initialized 被设置为 true。
 * 4. 成功解析后，若不是 JSON 对象则视为错误并重置为空对象。
 */
void AppConfig::loadInternal() const
{
    if (m_initialized)
    {
        return;
    }

    QFile file(m_filePath);
    if (!file.exists())
    {
        qInfo() << "Config file not found, will create default:" << m_filePath;
        // 确保目录结构存在，以便后续 save() 成功
        QDir().mkpath(QFileInfo(m_filePath).absolutePath());
        m_rootObject = QJsonObject();
        m_initialized = true;
        return;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot open config file for reading:" << m_filePath << file.errorString();
        m_rootObject = QJsonObject();
        m_initialized = true;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        qWarning() << "Config file parse error:" << parseError.errorString() << "at offset"
                   << parseError.offset;
        m_rootObject = QJsonObject();
        m_initialized = true;
        return;
    }

    if (!doc.isObject())
    {
        qWarning() << "Config file is not a JSON object, resetting.";
        m_rootObject = QJsonObject();
    }
    else
    {
        m_rootObject = doc.object();
    }
    m_initialized = true;
}

/**
 * @brief 重新加载配置，合并文件内容
 *
 * 若尚未初始化，则直接加载；否则读取文件并将顶层键逐一覆盖到内存对象。
 * 成功加载后发出 configReloaded 信号。
 */
bool AppConfig::load()
{
    QMutexLocker locker(&m_mutex);
    if (!m_initialized)
    {
        loadInternal();
        locker.unlock();
        emit configReloaded();
        return true;
    }

    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot open config file for reload:" << m_filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        qWarning() << "Reload parse error:" << parseError.errorString();
        return false;
    }

    if (!doc.isObject())
    {
        qWarning() << "Reloaded JSON is not an object.";
        return false;
    }

    QJsonObject newObj = doc.object();
    for (auto it = newObj.begin(); it != newObj.end(); ++it)
    {
        m_rootObject.insert(it.key(), it.value());
    }

    locker.unlock();
    emit configReloaded();
    return true;
}

/**
 * @brief 保存配置到磁盘（线程安全封装）
 */
bool AppConfig::save()
{
    QMutexLocker locker(&m_mutex);
    return writeToDisk();
}

/**
 * @brief 执行原子写入
 *
 * 使用 QSaveFile 将 m_rootObject 序列化为缩进 JSON 并写入。
 * 若文件写入成功且 commit 成功，则磁盘配置被更新。
 */
bool AppConfig::writeToDisk()
{
    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Cannot open config file for writing:" << m_filePath << file.errorString();
        return false;
    }

    QJsonDocument doc(m_rootObject);
    file.write(doc.toJson(QJsonDocument::Indented));
    if (!file.commit())
    {
        qWarning() << "Failed to commit config file:" << file.errorString();
        return false;
    }
    return true;
}

/**
 * @brief 读取配置值（QVariant 版本）
 *
 * 若键不存在且启用了自动写入默认值，会将默认值写入 JSON 树并发出信号。
 * 信号在解锁后发送，避免槽函数中调用配置接口导致死锁。
 */
QVariant AppConfig::value(const QString &key, const QVariant &defaultValue)
{
    QMutexLocker locker(&m_mutex);
    if (!m_initialized)
    {
        loadInternal();
    }

    QStringList parts = key.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    QVariant result = getValueFromObject(m_rootObject, parts);
    if (!result.isValid())
    {
        if (m_autoWriteDefault && defaultValue.isValid())
        {
            setValueInObject(m_rootObject, parts, defaultValue);
            locker.unlock();
            emit configChanged(key, defaultValue);
            return defaultValue;
        }
        return defaultValue;
    }
    return result;
}

/**
 * @brief 设置配置值
 *
 * 执行步骤：
 * 1. 确保已加载配置。
 * 2. 分割层级键。
 * 3. 检查值是否真正发生变化（避免无意义写入和信号）。
 * 4. 更新 JSON 树。
 * 5. 若需要自动保存，在锁内执行写入，保证保存内容与信号值一致。
 * 6. 解锁并发送信号。
 */
void AppConfig::setValue(const QString &key, const QVariant &val, bool autoSave)
{
    QMutexLocker locker(&m_mutex);
    if (!m_initialized)
    {
        loadInternal();
    }

    QStringList parts = key.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    if (parts.isEmpty())
    {
        qWarning("AppConfig::setValue: empty key");
        return;
    }

    QVariant oldVal = getValueFromObject(m_rootObject, parts);
    if (oldVal == val)
    {
        return;
    }

    setValueInObject(m_rootObject, parts, val);

    // 在释放锁之前执行保存，确保配置状态与信号一致
    if (autoSave || m_autoSaveEnabled)
    {
        writeToDisk();
    }

    locker.unlock();
    emit configChanged(key, val);
}

/**
 * @brief 删除键
 *
 * 删除成功后发送 configChanged 信号（值为无效 QVariant），表示键已移除。
 */
bool AppConfig::remove(const QString &key)
{
    QMutexLocker locker(&m_mutex);
    if (!m_initialized)
    {
        loadInternal();
    }

    QStringList parts = key.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    bool removed = removeKeyFromObject(m_rootObject, parts);
    if (removed)
    {
        locker.unlock();
        emit configChanged(key, QVariant());
    }
    return removed;
}

/**
 * @brief 检查键是否存在
 */
bool AppConfig::contains(const QString &key) const
{
    QMutexLocker locker(&m_mutex);
    if (!m_initialized)
    {
        loadInternal();
    }
    QStringList parts = key.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    return getValueFromObject(m_rootObject, parts).isValid();
}

/**
 * @brief 获取所有顶级组名
 */
QStringList AppConfig::childGroups() const
{
    QMutexLocker locker(&m_mutex);
    if (!m_initialized)
    {
        loadInternal();
    }
    QStringList groups;
    for (auto it = m_rootObject.begin(); it != m_rootObject.end(); ++it)
    {
        if (it.value().isObject())
        {
            groups.append(it.key());
        }
    }
    return groups;
}

/**
 * @brief 获取指定组下的所有叶子键（完整路径）
 *
 * 若 group 非空，先解析该组路径，若不存在或不是对象则返回空列表。
 */
QStringList AppConfig::childKeys(const QString &group) const
{
    QMutexLocker locker(&m_mutex);
    if (!m_initialized)
    {
        loadInternal();
    }

    const QJsonObject *target = &m_rootObject;
    if (!group.isEmpty())
    {
        QStringList groupParts = group.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        QVariant groupVal = getValueFromObject(m_rootObject, groupParts);
        if (!groupVal.isValid() || !groupVal.canConvert<QJsonObject>())
        {
            return QStringList();
        }
        QJsonObject groupObj = groupVal.value<QJsonObject>();
        return collectKeys(groupObj, QString(), true);
    }
    return collectKeys(*target, QString(), true);
}

// --- 以下为简单属性访问器 ---

void AppConfig::setAutoSaveEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_autoSaveEnabled = enabled;
}

bool AppConfig::isAutoSaveEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_autoSaveEnabled;
}

void AppConfig::setAutoWriteDefault(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_autoWriteDefault = enabled;
}

bool AppConfig::isAutoWriteDefault() const
{
    QMutexLocker locker(&m_mutex);
    return m_autoWriteDefault;
}

QString AppConfig::filePath() const
{
    QMutexLocker locker(&m_mutex);
    return m_filePath;
}

// ---------- 私有辅助方法 ----------

/**
 * @brief 通过路径片段列表从 JSON 对象中获取值
 *
 * 迭代遍历路径，每次取当前对象下的子键：
 * - 若值为 undefined，返回无效 QVariant。
 * - 若到达最后一个路径段，返回对应值。
 * - 若中间值不是对象，路径无法继续，返回无效 QVariant。
 *
 * @note 避免递归，使用循环减小栈开销。
 */
QVariant AppConfig::getValueFromObject(const QJsonObject &obj, const QStringList &pathParts) const
{
    if (pathParts.isEmpty())
    {
        return QVariant();
    }

    QJsonObject currentObj = obj;
    for (int i = 0; i < pathParts.size(); ++i)
    {
        QJsonValue val = currentObj.value(pathParts.at(i));
        if (val.isUndefined())
        {
            return QVariant();
        }
        if (i == pathParts.size() - 1)
        {
            return val.toVariant();
        }
        if (!val.isObject())
        {
            return QVariant();
        }
        currentObj = val.toObject();
    }
    return QVariant();
}

/**
 * @brief 递归将值设置到 JSON 对象中，必要时创建中间对象
 *
 * @param obj 要修改的 JSON 对象（引用）
 * @param pathParts 路径片段列表
 * @param val 新值（将转换为 QJsonValue 存储）
 */
void AppConfig::setValueInObject(QJsonObject &obj, const QStringList &pathParts,
                                 const QVariant &val)
{
    if (pathParts.isEmpty())
    {
        return;
    }
    if (pathParts.size() == 1)
    {
        obj.insert(pathParts.first(), QJsonValue::fromVariant(val));
        return;
    }

    QString childKey = pathParts.first();
    QJsonValue childVal = obj.value(childKey);
    QJsonObject childObj;
    if (childVal.isObject())
    {
        childObj = childVal.toObject();
    }
    else
    {
        childObj = QJsonObject();
    }

    setValueInObject(childObj, pathParts.mid(1), val);
    obj.insert(childKey, childObj);
}

/**
 * @brief 递归从 JSON 对象中删除指定键
 *
 * 仅当路径末端键存在时删除，父级中间对象即使变空也不会自动清理。
 *
 * @param obj 要操作的 JSON 对象（引用）
 * @param pathParts 路径片段列表
 * @return 是否成功删除
 */
bool AppConfig::removeKeyFromObject(QJsonObject &obj, const QStringList &pathParts)
{
    if (pathParts.isEmpty())
    {
        return false;
    }
    if (pathParts.size() == 1)
    {
        if (obj.contains(pathParts.first()))
        {
            obj.remove(pathParts.first());
            return true;
        }
        return false;
    }

    QString childKey = pathParts.first();
    QJsonValue childVal = obj.value(childKey);
    if (!childVal.isObject())
    {
        return false;
    }

    QJsonObject childObj = childVal.toObject();
    bool removed = removeKeyFromObject(childObj, pathParts.mid(1));
    if (removed)
    {
        obj.insert(childKey, childObj);
    }
    return removed;
}

/**
 * @brief 递归收集键列表
 *
 * @param obj 当前 JSON 对象
 * @param prefix 当前路径前缀（以 "/" 分隔）
 * @param keysOnly true 表示只收集叶子键（值非对象），false 也收集中间对象键
 * @return 键列表
 */
QStringList AppConfig::collectKeys(const QJsonObject &obj, const QString &prefix,
                                   bool keysOnly) const
{
    QStringList result;
    for (auto it = obj.begin(); it != obj.end(); ++it)
    {
        QString fullKey = prefix.isEmpty() ? it.key() : prefix + QLatin1Char('/') + it.key();
        if (it.value().isObject())
        {
            if (!keysOnly)
            {
                result.append(fullKey);
            }
            result.append(collectKeys(it.value().toObject(), fullKey, keysOnly));
        }
        else
        {
            if (keysOnly)
            {
                result.append(fullKey);
            }
        }
    }
    return result;
}