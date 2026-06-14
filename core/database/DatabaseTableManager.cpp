#include "DatabaseTableManager.h"
#include "Logger.h"
#include <QDebug>
#include <QRegularExpression>
#include <QSqlDriver>
#include <QThread>

/**
 * @brief 通用数据库表管理器，封装单表 CRUD 操作，支持事务和批量处理。
 *        使用参数化查询防止 SQL 注入，并对表名/列名进行白名单校验。
 */
DatabaseTableManager::DatabaseTableManager(const QString &dbPath, const QString &tableName,
                                           const QString &connectionName, QObject *parent)
    : QObject(parent)
    , m_tableName(tableName.trimmed())
{
    LOG_DEBUG_CAT(LogCategory::Database) << "DatabaseTableManager 构造，表名:" << m_tableName;

    if (!isValidIdentifier(m_tableName))
    {
        m_lastError = QStringLiteral("无效表名: %1 (仅允许字母、数字、下划线，且不能以数字开头)")
                          .arg(m_tableName);
        LOG_ERROR_CAT(LogCategory::Database) << m_lastError;
        return;
    }

    QString connName = connectionName;
    if (connName.isEmpty())
    {
        connName = QStringLiteral("DBMgr_%1_%2_%3")
        .arg(m_tableName)
            .arg(reinterpret_cast<quintptr>(this), 0, 16)
            .arg(reinterpret_cast<quintptr>(QThread::currentThread()), 0, 16);
    }

    if (QSqlDatabase::contains(connName))
    {
        m_db = QSqlDatabase::database(connName);
    }
    else
    {
        m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connName);
    }

    m_db.setDatabaseName(dbPath);
    if (!m_db.open())
    {
        m_lastError = m_db.lastError().text();
        LOG_ERROR_CAT(LogCategory::Database) << "打开数据库失败:" << m_lastError << " 路径:" << dbPath;
        return;
    }

    QSqlQuery pragma(m_db);
    pragma.exec(QStringLiteral("PRAGMA foreign_keys = ON"));
    pragma.exec(QStringLiteral("PRAGMA journal_mode = WAL"));
    LOG_INFO_CAT(LogCategory::Database) << "数据库连接成功，表名:" << m_tableName << " 路径:" << dbPath;
}

DatabaseTableManager::~DatabaseTableManager()
{
    if (m_db.isOpen())
    {
        QString connName = m_db.connectionName();
        m_db.close();
        m_db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connName);
        LOG_DEBUG_CAT(LogCategory::Database) << "数据库连接关闭:" << connName;
    }
}

bool DatabaseTableManager::checkDatabaseOpen()
{
    if (Q_UNLIKELY(!m_db.isOpen()))
    {
        m_lastError = QStringLiteral("数据库未打开");
        LOG_WARNING_CAT(LogCategory::Database) << m_lastError;
        return false;
    }
    return true;
}

bool DatabaseTableManager::executeQuery(QSqlQuery &query, const QString &errorContext)
{
    if (Q_UNLIKELY(!query.exec()))
    {
        m_lastError = errorContext + QStringLiteral(": ") + query.lastError().text();
        LOG_ERROR_CAT(LogCategory::Database) << m_lastError;
        return false;
    }
    return true;
}

QString DatabaseTableManager::validateColumns(const QStringList &columns)
{
    if (columns.isEmpty())
        return QStringLiteral("*");

    QStringList sanitized;
    for (const QString &col : columns)
    {
        if (isValidIdentifier(col))
            sanitized << col;
        else
            LOG_WARNING_CAT(LogCategory::Database) << "非法列名被忽略:" << col;
    }
    return sanitized.isEmpty() ? QString() : sanitized.join(QStringLiteral(", "));
}

bool DatabaseTableManager::isValidIdentifier(const QString &name)
{
    static const QRegularExpression reg(QStringLiteral("^[A-Za-z_][A-Za-z0-9_]*$"));
    return reg.match(name).hasMatch();
}

bool DatabaseTableManager::createTableIfNotExists(const QString &ddl)
{
    if (!checkDatabaseOpen())
        return false;

    QSqlQuery query(m_db);
    if (!query.exec(ddl))
    {
        m_lastError = query.lastError().text();
        LOG_ERROR_CAT(LogCategory::Database) << "创建表失败:" << m_lastError;
        return false;
    }

    LOG_DEBUG_CAT(LogCategory::Database) << "执行DDL成功:" << ddl.left(50) << "...";
    return true;
}

bool DatabaseTableManager::beginTransaction()
{
    if (!checkDatabaseOpen())
        return false;
    if (!m_db.transaction())
    {
        m_lastError = QStringLiteral("开始事务失败: ") + m_db.lastError().text();
        LOG_ERROR_CAT(LogCategory::Database) << m_lastError;
        return false;
    }
    LOG_DEBUG_CAT(LogCategory::Database) << "事务开始";
    return true;
}

bool DatabaseTableManager::commitTransaction()
{
    if (!checkDatabaseOpen())
        return false;
    if (!m_db.commit())
    {
        m_lastError = QStringLiteral("提交事务失败: ") + m_db.lastError().text();
        LOG_ERROR_CAT(LogCategory::Database) << m_lastError;
        return false;
    }
    LOG_DEBUG_CAT(LogCategory::Database) << "事务提交";
    return true;
}

bool DatabaseTableManager::rollbackTransaction()
{
    if (!checkDatabaseOpen())
        return false;
    if (!m_db.rollback())
    {
        m_lastError = QStringLiteral("回滚事务失败: ") + m_db.lastError().text();
        LOG_ERROR_CAT(LogCategory::Database) << m_lastError;
        return false;
    }
    LOG_DEBUG_CAT(LogCategory::Database) << "事务回滚";
    return true;
}

qint64 DatabaseTableManager::insert(const QVariantMap &data)
{
    if (!checkDatabaseOpen())
        return -1;
    if (data.isEmpty())
    {
        m_lastError = QStringLiteral("插入数据为空");
        LOG_WARNING_CAT(LogCategory::Database) << m_lastError;
        return -1;
    }

    QStringList columns, placeholders;
    QVariantList values;
    for (auto it = data.cbegin(); it != data.cend(); ++it)
    {
        if (!isValidIdentifier(it.key()))
        {
            m_lastError = QStringLiteral("无效列名: %1").arg(it.key());
            LOG_ERROR_CAT(LogCategory::Database) << m_lastError;
            return -1;
        }
        columns << it.key();
        placeholders << QStringLiteral("?");
        values << it.value();
    }

    QString sql = QStringLiteral("INSERT INTO %1 (%2) VALUES (%3)")
                      .arg(m_tableName, columns.join(QStringLiteral(", ")),
                           placeholders.join(QStringLiteral(", ")));

    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const QVariant &val : qAsConst(values))
        query.addBindValue(val);

    if (!executeQuery(query, QStringLiteral("插入数据失败")))
        return -1;

    qint64 id = query.lastInsertId().toLongLong();
    LOG_DEBUG_CAT(LogCategory::Database) << "插入行成功, id:" << id;
    return id;
}

int DatabaseTableManager::update(const QVariantMap &data, const QString &whereClause,
                                 const QVariantList &whereArgs)
{
    if (!checkDatabaseOpen())
        return -1;
    if (data.isEmpty())
        return 0;

    QStringList setClauses;
    QVariantList values;
    for (auto it = data.cbegin(); it != data.cend(); ++it)
    {
        if (!isValidIdentifier(it.key()))
        {
            m_lastError = QStringLiteral("无效列名: %1").arg(it.key());
            LOG_ERROR_CAT(LogCategory::Database) << m_lastError;
            return -1;
        }
        setClauses << QStringLiteral("%1 = ?").arg(it.key());
        values << it.value();
    }

    QString sql = QStringLiteral("UPDATE %1 SET %2").arg(m_tableName, setClauses.join(QStringLiteral(", ")));
    if (!whereClause.isEmpty())
    {
        sql += QStringLiteral(" WHERE ") + whereClause;
        values.append(whereArgs);
    }

    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const QVariant &val : qAsConst(values))
        query.addBindValue(val);

    if (!executeQuery(query, QStringLiteral("更新数据失败")))
        return -1;
    int affected = query.numRowsAffected();
    LOG_DEBUG_CAT(LogCategory::Database) << "更新行数:" << affected;
    return affected;
}

int DatabaseTableManager::remove(const QString &whereClause, const QVariantList &whereArgs)
{
    if (!checkDatabaseOpen())
        return -1;

    QString sql = QStringLiteral("DELETE FROM %1").arg(m_tableName);
    if (!whereClause.isEmpty())
        sql += QStringLiteral(" WHERE ") + whereClause;

    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const QVariant &val : whereArgs)
        query.addBindValue(val);

    if (!executeQuery(query, QStringLiteral("删除数据失败")))
        return -1;
    int affected = query.numRowsAffected();
    LOG_DEBUG_CAT(LogCategory::Database) << "删除行数:" << affected;
    return affected;
}

QVariantMap DatabaseTableManager::selectOne(const QStringList &columns, const QString &whereClause,
                                            const QVariantList &whereArgs, const QString &orderBy)
{
    QList<QVariantMap> rows = select(columns, whereClause, whereArgs, orderBy, 1);
    return rows.isEmpty() ? QVariantMap() : rows.first();
}

QList<QVariantMap> DatabaseTableManager::select(const QStringList &columns,
                                                const QString &whereClause,
                                                const QVariantList &whereArgs,
                                                const QString &orderBy, int limit)
{
    QList<QVariantMap> results;
    if (!checkDatabaseOpen())
        return results;

    QString cols = validateColumns(columns);
    if (cols.isEmpty() && !columns.isEmpty())
    {
        m_lastError = QStringLiteral("所有指定列名均无效");
        LOG_WARNING_CAT(LogCategory::Database) << m_lastError;
        return results;
    }

    QString sql = QStringLiteral("SELECT %1 FROM %2").arg(cols, m_tableName);
    if (!whereClause.isEmpty())
        sql += QStringLiteral(" WHERE ") + whereClause;
    if (!orderBy.isEmpty())
        sql += QStringLiteral(" ORDER BY ") + orderBy;
    if (limit > 0)
        sql += QStringLiteral(" LIMIT ") + QString::number(limit);

    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const QVariant &val : whereArgs)
        query.addBindValue(val);

    if (!executeQuery(query, QStringLiteral("查询数据失败")))
        return results;

    QSqlRecord record = query.record();
    while (query.next())
    {
        QVariantMap row;
        for (int i = 0; i < record.count(); ++i)
            row[record.fieldName(i)] = query.value(i);
        results.append(row);
    }
    LOG_DEBUG_CAT(LogCategory::Database) << "查询返回行数:" << results.size();
    return results;
}

int DatabaseTableManager::count(const QString &whereClause, const QVariantList &whereArgs)
{
    if (!checkDatabaseOpen())
        return -1;

    QString sql = QStringLiteral("SELECT COUNT(*) FROM %1").arg(m_tableName);
    if (!whereClause.isEmpty())
        sql += QStringLiteral(" WHERE ") + whereClause;

    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const QVariant &val : whereArgs)
        query.addBindValue(val);

    if (!executeQuery(query, QStringLiteral("统计行数失败")))
        return -1;
    if (query.next())
        return query.value(0).toInt();
    return -1;
}

int DatabaseTableManager::insertBatch(const QList<QVariantMap> &dataList)
{
    if (!checkDatabaseOpen())
        return -1;
    if (dataList.isEmpty())
        return 0;

    LOG_DEBUG_CAT(LogCategory::Database) << "开始批量插入，条数:" << dataList.size();

    if (!beginTransaction())
        return -1;

    int success = 0;
    for (const QVariantMap &row : dataList)
    {
        qint64 id = insert(row);
        if (id < 0)
        {
            rollbackTransaction();
            return -1;
        }
        success++;
    }

    if (!commitTransaction())
    {
        rollbackTransaction();
        return -1;
    }
    LOG_DEBUG_CAT(LogCategory::Database) << "批量插入成功，插入行数:" << success;
    return success;
}