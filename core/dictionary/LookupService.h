#ifndef LOOKUPSERVICE_H
#define LOOKUPSERVICE_H

#include <QObject>
#include <QSqlDatabase>
#include "../common/WordEntry.h"

/**
 * @brief 词典查询服务，从 SQLite 数据库中查找单词释义。
 */
class LookupService : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数。
     * @param db 已打开的数据库连接，用于查询词典表。
     * @param parent QObject 父对象指针。
     */
    explicit LookupService(const QSqlDatabase &db, QObject *parent = nullptr);
    ~LookupService();

    /**
     * @brief 查询指定单词的释义信息。
     * @param word 待查询的单词（大小写不敏感）。
     * @return 包含单词、音标、翻译的 WordEntry 结构体；若未找到则 word 字段为空。
     */
    WordEntry lookupWord(const QString &word) const;

private:
    QSqlDatabase m_db;   ///< 数据库连接实例。
};

#endif // LOOKUPSERVICE_H