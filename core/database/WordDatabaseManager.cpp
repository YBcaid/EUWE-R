#include "WordDatabaseManager.h"
#include "TextUtils.h"
#include "Logger.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

/**
 * @brief 熟词库数据管理器。继承通用数据库表管理器，专门管理单词表，
 *        提供单词的增删改查、批量操作和事务支持。
 */
WordDatabaseManager::WordDatabaseManager(const QString &dbPath, const QString &tableName,
                                         const QString &connectionName, QObject *parent)
    : DatabaseTableManager(dbPath, tableName, connectionName, parent)
{
    if (!isOpen())
    {
        LOG_ERROR_CAT(LogCategory::Database) << "WordDatabaseManager 数据库未打开";
        return;
    }

    const QString ddl = QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
                                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                       "word TEXT NOT NULL UNIQUE,"
                                       "translation TEXT,"
                                       "phonetic TEXT)")
                            .arg(m_tableName);

    if (!createTableIfNotExists(ddl))
    {
        LOG_ERROR_CAT(LogCategory::Database) << "创建 Words 表失败:" << lastError();
    }
    else
    {
        LOG_INFO_CAT(LogCategory::Database) << "Words 表已就绪，表名:" << m_tableName;
    }
}

QVariantMap WordDatabaseManager::wordToMap(const WordEntry &word)
{
    QVariantMap map;
    map[QStringLiteral("word")] = word.word;
    map[QStringLiteral("translation")] = word.translation;
    map[QStringLiteral("phonetic")] = word.phonetic;
    return map;
}

WordEntry WordDatabaseManager::mapToWord(const QVariantMap &map)
{
    WordEntry w;
    w.word = map.value(QStringLiteral("word")).toString();
    w.translation = map.value(QStringLiteral("translation")).toString();
    w.phonetic = map.value(QStringLiteral("phonetic")).toString();
    return w;
}

bool WordDatabaseManager::containsWord(const QString &word)
{
    if (!checkDatabaseOpen())
        return false;
    QString norm = normalizeWord(word);
    if (norm.isEmpty())
        return false;

    int cnt = count(QStringLiteral("word = ?"), {norm});
    bool exists = cnt > 0;
    LOG_DEBUG_CAT(LogCategory::Database) << "检查单词是否存在:" << norm << " -> " << exists;
    return exists;
}

WordEntry WordDatabaseManager::getWordInfo(const QString &word)
{
    if (!checkDatabaseOpen())
        return WordEntry();
    QString norm = normalizeWord(word);
    if (norm.isEmpty())
        return WordEntry();

    QVariantMap row = selectOne(
        {QStringLiteral("word"), QStringLiteral("translation"), QStringLiteral("phonetic")},
        QStringLiteral("word = ?"), {norm});
    WordEntry entry = mapToWord(row);
    if (!entry.word.isEmpty())
        LOG_DEBUG_CAT(LogCategory::Database) << "查询单词:" << norm << " 释义:" << entry.translation;
    else
        LOG_DEBUG_CAT(LogCategory::Database) << "查询单词:" << norm << " 未找到";
    return entry;
}

QList<WordEntry> WordDatabaseManager::getAllWords()
{
    QList<WordEntry> words;
    if (!checkDatabaseOpen())
        return words;

    QList<QVariantMap> rows =
        select({QStringLiteral("word"), QStringLiteral("translation"), QStringLiteral("phonetic")},
               QString(), QVariantList(), QStringLiteral("word"));
    for (const QVariantMap &row : rows)
    {
        words.append(mapToWord(row));
    }
    LOG_INFO_CAT(LogCategory::Database) << "加载所有熟词，共" << words.size() << "条";
    return words;
}

bool WordDatabaseManager::addWord(const QString &word, const QString &translation,
                                  const QString &phonetic)
{
    if (!checkDatabaseOpen())
        return false;

    QString norm = normalizeWord(word);
    if (norm.isEmpty())
    {
        m_lastError = QStringLiteral("单词无效：规范化后为空");
        LOG_WARNING_CAT(LogCategory::Database) << "添加单词失败: 规范化后为空 (原始:" << word << ")";
        return false;
    }

    QSqlQuery query(m_db);
    QString sql = QStringLiteral("INSERT OR IGNORE INTO %1 (word, translation, phonetic) "
                                 "VALUES (?, ?, ?)")
                      .arg(m_tableName);
    query.prepare(sql);
    query.addBindValue(norm);
    query.addBindValue(translation);
    query.addBindValue(phonetic);

    if (!executeQuery(query, QStringLiteral("添加单词失败")))
        return false;

    LOG_INFO_CAT(LogCategory::Database) << "添加单词:" << norm << " 释义:" << translation;
    return true;
}

int WordDatabaseManager::addWords(const QList<WordEntry> &words)
{
    if (!checkDatabaseOpen())
        return -1;
    if (words.isEmpty())
        return 0;

    LOG_INFO_CAT(LogCategory::Database) << "开始批量添加单词，总数:" << words.size();

    if (!beginTransaction())
        return -1;

    QSqlQuery query(m_db);
    QString sql = QStringLiteral("INSERT OR IGNORE INTO %1 (word, translation, phonetic) "
                                 "VALUES (?, ?, ?)")
                      .arg(m_tableName);
    query.prepare(sql);

    int inserted = 0;
    for (const WordEntry &w : words)
    {
        QString norm = normalizeWord(w.word);
        if (norm.isEmpty())
            continue;

        query.addBindValue(norm);
        query.addBindValue(w.translation);
        query.addBindValue(w.phonetic);

        if (!query.exec())
        {
            m_lastError = QStringLiteral("批量添加失败: ") + query.lastError().text();
            LOG_ERROR_CAT(LogCategory::Database) << m_lastError;
            rollbackTransaction();
            return -1;
        }

        if (query.numRowsAffected() > 0)
        {
            ++inserted;
            LOG_DEBUG_CAT(LogCategory::Database) << "批量添加成功:" << norm;
        }
        else
        {
            LOG_DEBUG_CAT(LogCategory::Database) << "批量添加忽略(已存在):" << norm;
        }
    }

    if (!commitTransaction())
    {
        m_lastError = QStringLiteral("提交事务失败: ") + m_db.lastError().text();
        LOG_ERROR_CAT(LogCategory::Database) << m_lastError;
        rollbackTransaction();
        return -1;
    }

    LOG_INFO_CAT(LogCategory::Database) << "批量添加完成，实际新增:" << inserted;
    return inserted;
}

bool WordDatabaseManager::removeWord(const QString &word)
{
    if (!checkDatabaseOpen())
        return false;

    QString norm = normalizeWord(word);
    if (norm.isEmpty())
        return true;

    int rows = remove(QStringLiteral("word = ?"), {norm});
    bool success = rows >= 0;
    if (success)
        LOG_INFO_CAT(LogCategory::Database) << "删除单词:" << norm;
    else
        LOG_ERROR_CAT(LogCategory::Database) << "删除单词失败:" << norm;
    return success;
}

int WordDatabaseManager::removeWords(const QStringList &words)
{
    if (!checkDatabaseOpen())
        return -1;
    if (words.isEmpty())
        return 0;

    LOG_INFO_CAT(LogCategory::Database) << "开始批量删除单词，总数:" << words.size();

    if (!beginTransaction())
        return -1;

    QSqlQuery query(m_db);
    QString sql = QStringLiteral("DELETE FROM %1 WHERE word = ?").arg(m_tableName);

    int deleted = 0;
    for (const QString &word : words)
    {
        QString norm = normalizeWord(word);
        if (norm.isEmpty())
            continue;

        query.prepare(sql);
        query.addBindValue(norm);

        if (!query.exec())
        {
            m_lastError = QStringLiteral("批量删除失败: ") + query.lastError().text();
            LOG_ERROR_CAT(LogCategory::Database) << m_lastError;
            rollbackTransaction();
            return -1;
        }
        int affected = query.numRowsAffected();
        deleted += affected;
        if (affected > 0)
            LOG_DEBUG_CAT(LogCategory::Database) << "批量删除成功:" << norm;
        else
            LOG_DEBUG_CAT(LogCategory::Database) << "批量删除忽略(不存在):" << norm;
    }

    if (!commitTransaction())
    {
        m_lastError = QStringLiteral("提交事务失败: ") + m_db.lastError().text();
        LOG_ERROR_CAT(LogCategory::Database) << m_lastError;
        rollbackTransaction();
        return -1;
    }

    LOG_INFO_CAT(LogCategory::Database) << "批量删除完成，实际删除:" << deleted;
    return deleted;
}

QString WordDatabaseManager::normalizeWord(const QString &text)
{
    return TextUtils::keepOnlyEnglishLetters(text);
}