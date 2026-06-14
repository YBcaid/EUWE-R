// appconfig.h
#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QStringList>
#include <QVariant>

/**
 * @brief 应用程序配置管理类（单例模式，线程安全）
 *
 * 基于 JSON 文件持久化存储，支持层级键（如 "group/subgroup/key"）。
 * 提供类型安全的读写接口，具备默认值、自动保存、脏标记等机制。
 * 惰性加载：首次访问配置或显式调用 load() 时从磁盘读取。
 *
 * 线程安全设计：
 * - 所有公有方法均通过 QMutex 保护内部状态，可在多线程中安全调用。
 * - 信号在锁释放后发送，避免死锁。
 * - 使用 mutable 成员允许 const 方法触发惰性加载，同时保持 const 语义。
 *
 * @note 使用前应调用 initialize() 指定配置文件路径（可选），否则使用默认路径。
 * @warning 不要在信号槽中同步调用可能再次获取锁的接口，避免死锁（Qt 支持
 *          Qt::QueuedConnection 解决跨线程问题）。
 */
class AppConfig : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(AppConfig)

    public:
    /**
     * @brief 获取单例实例
     * @return AppConfig 单例引用（非 const，允许修改配置）
     *
     * 首次调用时触发惰性加载，从磁盘读取 JSON 配置文件。
     * 若加载失败（文件不存在、解析错误），将持有空对象，不抛出异常。
     */
    static AppConfig &instance();

    /**
     * @brief 初始化配置文件路径（必须在首次真正使用前调用）
     * @param filePath 完整的 JSON 配置文件路径，若为空则使用默认路径
     *
     * 默认路径为 QStandardPaths::writableLocation(AppDataLocation) + "/config.json"
     * 若实例已加载配置，路径变更将被忽略并输出警告。
     */
    static void initialize(const QString &filePath = QString());

    /**
     * @brief 从磁盘重新加载配置，合并到现有数据
     * @return 成功返回 true
     *
     * 加载策略：
     * - 若尚未初始化，直接加载（等同于首次惰性加载）。
     * - 若已初始化，读取文件并合并到内存对象：顶层键存在则覆盖，不存在则添加；
     *   嵌套对象整体替换，不做深度合并。
     * - 成功后发出 configReloaded 信号。
     */
    bool load();

    /**
     * @brief 将当前配置保存到磁盘
     * @return 成功返回 true
     *
     * 使用 QSaveFile 原子写入，避免断电导致配置损坏。
     */
    bool save();

    /**
     * @brief 获取配置值（模板版本，类型安全）
     * @param key 层级键，例如 "network/timeout"
     * @param defaultValue 当键不存在或类型转换失败时返回的默认值
     * @return 配置值，若无法获取则返回 defaultValue
     *
     * 内部通过 QVariant 转换，若转换失败则返回原始默认值。
     * 如果启用了 setAutoWriteDefault(true)，键缺失且默认值有效，
     * 会将默认值写入配置并发出 configChanged 信号。
     */
    template <typename T> T value(const QString &key, const T &defaultValue = T());

    /**
     * @brief 获取配置值（QVariant 版本）
     * @param key 层级键
     * @param defaultValue 默认值
     * @return 配置值或默认值
     */
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant());

    /**
     * @brief 设置配置值
     * @param key 层级键（禁止为空，否则输出警告并忽略）
     * @param val 新值
     * @param autoSave 是否立即保存到磁盘（覆盖全局自动保存设置）
     *
     * 若新旧值相同则不执行任何操作，也不发送信号。
     * 成功修改后发出 configChanged 信号。
     */
    void setValue(const QString &key, const QVariant &val, bool autoSave = false);

    /**
     * @brief 删除指定键及其子键
     * @param key 层级键
     * @return 如果键存在并成功删除返回 true
     *
     * 成功删除后发出 configChanged 信号（值为无效 QVariant 表示删除）。
     */
    bool remove(const QString &key);

    /**
     * @brief 检查键是否存在（即路径指向有效值，包括 null 值）
     * @param key 层级键
     * @return 存在返回 true
     */
    bool contains(const QString &key) const;

    /**
     * @brief 返回所有顶级组名（值为 JSON 对象的键）
     * @return 组名列表
     */
    QStringList childGroups() const;

    /**
     * @brief 返回指定组下的所有叶子键（完整路径）
     * @param group 组路径（空字符串表示顶层）
     * @return 叶子键列表（仅叶节点，不包含中间对象键）
     */
    QStringList childKeys(const QString &group = QString()) const;

    /**
     * @brief 设置是否在每次 setValue 后自动保存（默认 false）
     */
    void setAutoSaveEnabled(bool enabled);

    /**
     * @brief 查询自动保存是否启用
     */
    [[nodiscard]] bool isAutoSaveEnabled() const;

    /**
     * @brief 设置读取不存在的键时，是否自动将默认值写入配置（默认 false）
     */
    void setAutoWriteDefault(bool enabled);

    /**
     * @brief 查询自动写入默认值是否启用
     */
    [[nodiscard]] bool isAutoWriteDefault() const;

    /**
     * @brief 返回当前配置文件完整路径
     */
    [[nodiscard]] QString filePath() const;

    /**
     * @brief 析构函数（公开，允许 QScopedPointer 正确释放）
     *
     * 析构时尝试保存配置（若已初始化），释放所有资源。
     */
    ~AppConfig() override;

    signals:
    /**
     * @brief 配置变更信号
     * @param key 变更的键
     * @param newValue 新值（删除时为无效 QVariant）
     *
     * 仅在值实际发生改变时发出。信号在锁释放后发送，槽函数中应避免
     * 长时间阻塞或同步调用本类公有接口（可能导致递归锁定等待）。
     */
    void configChanged(const QString &key, const QVariant &newValue);

    /**
     * @brief 配置从磁盘重新加载成功信号
     *
     * 在 load() 成功后发出，可用于刷新 UI 或模块整体状态。
     */
    void configReloaded();

    private:
    /**
     * @brief 私有构造函数，防止外部直接创建实例
     *
     * 构造时设置默认配置文件路径，但不会加载文件。
     */
    AppConfig();

    // 静态单例相关
    static QScopedPointer<AppConfig> s_instance; ///< 单例智能指针
    static QMutex s_instanceMutex;				 ///< 保护单例创建的互斥锁

    /**
     * @brief 内部加载实现（必须持有 m_mutex）
     *
     * 从磁盘读取并解析 JSON，初始化 m_rootObject。
     * 任何读取失败均会初始化为空对象，保证 m_initialized 最终为 true。
     */
    void loadInternal() const;

    /**
     * @brief 原子写入磁盘（必须持有 m_mutex）
     * @return 成功返回 true
     *
     * 使用 QSaveFile 进行原子写入，先写入临时文件再替换。
     */
    bool writeToDisk();

    /**
     * @brief 从 JSON 对象中按路径递归获取值
     * @param obj 起始 JSON 对象
     * @param pathParts 路径片段列表
     * @return 找到的值，若路径不存在或中间非对象则返回无效 QVariant
     */
    QVariant getValueFromObject(const QJsonObject &obj, const QStringList &pathParts) const;

    /**
     * @brief 将值递归设置到 JSON 对象的指定路径
     * @param obj 要修改的 JSON 对象（引用）
     * @param pathParts 路径片段列表
     * @param val 新值
     *
     * 如果路径中某个中间节点不是对象，会创建空对象替换。
     */
    static void setValueInObject(QJsonObject &obj, const QStringList &pathParts,
                                 const QVariant &val);

    /**
     * @brief 从 JSON 对象中递归删除指定路径的键
     * @param obj 要修改的 JSON 对象（引用）
     * @param pathParts 路径片段列表
     * @return 成功删除返回 true
     *
     * 只会删除叶子键，中间对象不会因变空而自动清理。
     */
    static bool removeKeyFromObject(QJsonObject &obj, const QStringList &pathParts);

    /**
     * @brief 递归收集键列表
     * @param obj 起始 JSON 对象
     * @param prefix 当前路径前缀
     * @param keysOnly true 只收集叶子键，false 也收集对象键作为组
     * @return 键列表
     */
    QStringList collectKeys(const QJsonObject &obj, const QString &prefix, bool keysOnly) const;

    // ---- 成员变量 ----

    QString m_filePath;					///< 配置文件完整路径
    mutable QJsonObject m_rootObject;	///< 内存中的 JSON 根对象（mutable 允许 const 方法惰性加载）
    bool m_autoSaveEnabled = false;		///< 全局自动保存标志
    bool m_autoWriteDefault = false;	///< 全局自动写入默认值标志
    mutable QMutex m_mutex;				///< 保护所有成员变量的互斥锁（mutable 用于 const 方法加锁）
    mutable bool m_initialized = false; ///< 是否已完成首次加载（mutable 允许 const 方法修改）
};

// 模板函数实现（必须在头文件中）
template <typename T> T AppConfig::value(const QString &key, const T &defaultValue)
{
    QVariant var = value(key, QVariant::fromValue(defaultValue));
    // 如果 QVariant 可转换为目标类型，则转换并返回；否则返回原始默认值
    if (var.canConvert<T>())
    {
        return var.value<T>();
    }
    return defaultValue;
}

#endif // APPCONFIG_H