#include "LookupService.h"
#include "Logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

LookupService::LookupService(const QSqlDatabase &db, QObject *parent)
    : QObject(parent), m_db(db)
{
    if (!m_db.isOpen())
        LOG_WARNING_CAT(LogCategory::Database) << "LookupService: 数据库未打开";
    else
        LOG_DEBUG_CAT(LogCategory::Database) << "LookupService 已初始化";
}

LookupService::~LookupService() = default;

WordEntry LookupService::lookupWord(const QString &word) const
{
    if (!m_db.isOpen())
    {
        LOG_WARNING_CAT(LogCategory::Database) << "LookupService: 数据库未打开，无法查询单词:" << word;
        return WordEntry();
    }

    QString norm = word.toLower().trimmed();

    QSqlQuery query(m_db);
    query.prepare("SELECT word, phonetic, translation FROM stardict WHERE word = ? COLLATE NOCASE");
    query.addBindValue(norm);

    if (!query.exec())
    {
        LOG_ERROR_CAT(LogCategory::Database) << "LookupService 查询失败:" << query.lastError().text();
        return WordEntry();
    }

    if (query.next())
    {
        WordEntry entry;
        entry.word = query.value(0).toString();
        entry.phonetic = query.value(1).toString();
        entry.translation = query.value(2).toString();
        LOG_DEBUG_CAT(LogCategory::Database) << "单词查询成功:" << norm << " -> " << entry.translation;
        return entry;
    }
    else
    {
        LOG_DEBUG_CAT(LogCategory::Database) << "单词未找到:" << norm;
        return WordEntry();
    }
}