// DatabaseTableManager.h
#ifndef DATABASETABLEMANAGER_H
#define DATABASETABLEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

/**
 * @brief 通用数据库表管理器（数据访问层基类 / Gateway）
 *
 * 封装了基于 QSqlDatabase 的单表 CRUD 操作，支持事务、批量处理、错误缓存。
 * 通过参数化查询防范 SQL 注入，表名与列名均进行白名单校验。
 *
 * 设计意图：
 * - 可作为基类，通过继承添加特定业务逻辑；
 * - 也可直接实例化，通过传入表名与列定义，快速操作任意单表。
 *
 * 使用步骤：
 * 1. 实例化时指定数据库路径、表名、连接名；
 * 2. 调用 createTableIfNotExists(ddl) 确保表结构存在；
 * 3. 使用 insert/update/remove/select 等方法进行数据操作。
 *
 * 线程安全：每个实例应在创建它的线程中使用（Qt 对象线程亲和性）。
 */
class DatabaseTableManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DatabaseTableManager)

    public:
    /**
     * @brief 构造函数
     * @param dbPath         SQLite 数据库文件路径
     * @param tableName      目标表名（只允许字母、数字、下划线，且不能以数字开头）
     * @param connectionName Qt 数据库连接名（留空则自动生成唯一名）
     * @param parent         QObject 父对象
     */
    explicit DatabaseTableManager(const QString &dbPath, const QString &tableName,
                                  const QString &connectionName = QString(),
                                  QObject *parent = nullptr);

    ~DatabaseTableManager() override;

    /// 数据库是否已成功打开
    bool isOpen() const
    {
        return m_db.isOpen();
    }

    /// 获取最近一次错误信息
    QString lastError() const
    {
        return m_lastError;
    }

    // ---------- 表结构管理 ----------
    /**
     * @brief 执行 DDL 创建表（仅当表不存在时）
     * @param ddl 完整的 CREATE TABLE IF NOT EXISTS ... 语句
     * @return 执行成功返回 true
     */
    bool createTableIfNotExists(const QString &ddl);

    // ---------- 事务管理 ----------
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // ---------- CRUD 基础操作 ----------
    /**
     * @brief 插入一行数据
     * @param data 列名 -> 值的映射；键名必须为有效的列名
     * @return 新插入行的 rowid（主键），失败返回 -1
     */
    qint64 insert(const QVariantMap &data);

    /**
     * @brief 根据条件更新数据
     * @param data         要更新的列值映射
     * @param whereClause  WHERE 子句（不含 "WHERE"），使用 ? 占位符，例如 "id = ?"
     * @param whereArgs    WHERE 子句占位符对应的参数列表
     * @return 受影响的行数，失败返回 -1
     */
    int update(const QVariantMap &data, const QString &whereClause,
               const QVariantList &whereArgs = QVariantList());

    /**
     * @brief 根据条件删除数据
     * @param whereClause  WHERE 子句（不含 "WHERE"）
     * @param whereArgs    参数列表
     * @return 删除的行数，失败返回 -1
     */
    int remove(const QString &whereClause, const QVariantList &whereArgs = QVariantList());

    /**
     * @brief 查询单行数据
     * @param columns      需要返回的列名列表；为空则返回所有列
     * @param whereClause  WHERE 条件
     * @param whereArgs    参数列表
     * @param orderBy      ORDER BY 子句（不含 "ORDER BY"），可选
     * @return 查询到的第一行数据，若无结果则返回空 QVariantMap
     */
    QVariantMap selectOne(const QStringList &columns = QStringList(),
                          const QString &whereClause = QString(),
                          const QVariantList &whereArgs = QVariantList(),
                          const QString &orderBy = QString());

    /**
     * @brief 查询多行数据
     * @param columns      需要返回的列名列表；为空则返回所有列
     * @param whereClause  WHERE 条件
     * @param whereArgs    参数列表
     * @param orderBy      ORDER BY 子句
     * @param limit        最大返回行数，<=0 表示不限制
     * @return 结果行列表，每行为 QVariantMap
     */
    QList<QVariantMap> select(const QStringList &columns = QStringList(),
                              const QString &whereClause = QString(),
                              const QVariantList &whereArgs = QVariantList(),
                              const QString &orderBy = QString(), int limit = 0);

    /**
     * @brief 统计行数
     * @param whereClause  WHERE 条件
     * @param whereArgs    参数列表
     * @return 行数，失败返回 -1
     */
    int count(const QString &whereClause = QString(),
              const QVariantList &whereArgs = QVariantList());

    // ---------- 批量操作 ----------
    /**
     * @brief 批量插入（使用事务）
     * @param dataList 多行数据列表
     * @return 成功插入的行数，若事务内发生错误则返回 -1
     */
    int insertBatch(const QList<QVariantMap> &dataList);

    // ---------- 工具函数 ----------
    /**
     * @brief 验证标识符（表名、列名）是否安全
     * @return 仅包含字母、数字、下划线且不以数字开头则为 true
     */
    static bool isValidIdentifier(const QString &name);

    protected:
    /// 执行已准备好的查询，自动记录错误
    bool executeQuery(QSqlQuery &query, const QString &errorContext);

    /// 检查数据库是否打开，未打开则设置错误
    bool checkDatabaseOpen();

    /// 验证并返回合法的列名字符串，用于拼接 SQL；若验证失败返回空字符串
    QString validateColumns(const QStringList &columns);

    QSqlDatabase m_db;	 ///< 数据库连接
    QString m_tableName; ///< 受管理的表名（已经过验证）
    QString m_lastError; ///< 最后一次错误信息
};

#endif // DATABASETABLEMANAGER_H