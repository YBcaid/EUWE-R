// WordDatabaseManager.h
#ifndef WORDDATABASEMANAGER_H
#define WORDDATABASEMANAGER_H

#include "DatabaseTableManager.h"
#include "WordEntry.h"
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

/**
 * @brief 熟词库数据管理器
 *
 * 继承通用数据库表管理器 DatabaseTableManager，专门管理单词表（Words）。
 * 提供针对单词的增删改查、批量操作、事务支持与错误缓存。
 * 所有单词在存储和查询前均通过 normalizeWord() 进行规范化（小写、仅保留英文字母和连字符），
 * 实现大小写不敏感和符号无关的比较。
 *
 * 线程安全性：每个实例应在创建它的线程中使用（Qt 对象线程亲和性）。
 */
class WordDatabaseManager : public DatabaseTableManager
{
    Q_OBJECT
    Q_DISABLE_COPY(WordDatabaseManager)

    public:
    /**
     * @brief 构造函数
     * @param dbPath 数据库文件路径，默认 "euwe_words.db"
     * @param tableName 表名，默认 "Words"（需符合标识符规范）
     * @param connectionName 数据库连接名，留空则自动生成唯一名称
     * @param parent QObject 父对象
     */
    explicit WordDatabaseManager(const QString &dbPath = QStringLiteral("euwe_words.db"),
                                 const QString &tableName = QStringLiteral("Words"),
                                 const QString &connectionName = QString(),
                                 QObject *parent = nullptr);

    // ──────── 单词查询 ────────

    /**
     * @brief 检查单词是否已存在于熟词库
     * @param word 待检查的单词（自动规范化）
     * @return true 存在；false 不存在或数据库未打开
     */
    bool containsWord(const QString &word);

    /**
     * @brief 获取单词的完整信息
     * @param word 待查询的单词（自动规范化）
     * @return Word 结构体；若单词不存在则返回 word 字段为空的 Word 对象
     */
    WordEntry getWordInfo(const QString &word);

    /**
     * @brief 获取熟词库中的所有单词
     * @return Word 列表，按单词字典序排序；数据库未打开时返回空列表
     */
    QList<WordEntry> getAllWords();

    // ──────── 单词插入 ────────

    /**
     * @brief 添加单个单词（若已存在则忽略）
     * @param word 单词（自动规范化）
     * @param translation 释义
     * @param phonetic 音标
     * @return true 操作成功（插入或已存在）；false 数据库错误或无效单词
     */
    bool addWord(const QString &word, const QString &translation, const QString &phonetic);

    /**
     * @brief 批量添加单词（使用事务）
     * @param words Word 列表
     * @return 实际新插入的单词数量；若事务内发生数据库错误则返回 -1
     */
    int addWords(const QList<WordEntry> &words);

    // ──────── 单词删除 ────────

    /**
     * @brief 删除单个单词
     * @param word 待删除的单词（自动规范化）
     * @return true 删除成功或单词原不存在；false 数据库错误
     */
    bool removeWord(const QString &word);

    /**
     * @brief 批量删除单词（使用事务）
     * @param words 单词列表
     * @return 成功删除的单词数量；若事务内发生数据库错误则返回 -1
     */
    int removeWords(const QStringList &words);

    // ──────── 工具函数 ────────

    /**
     * @brief 规范化单词字符串
     *
     * 仅保留英文字母（含部分扩展拉丁字母）和连字符 '-'，并转换为小写，
     * 用于大小写不敏感比较和符号无关匹配。
     * @param text 原始文本
     * @return 规范化后的字符串（可能为空）
     */
    static QString normalizeWord(const QString &text);

    private:
    /**
     * @brief 将 Word 结构体转换为 QVariantMap，便于与基类通用接口交互
     */
    static QVariantMap wordToMap(const WordEntry &word);

    /**
     * @brief 将 QVariantMap 转换为 Word 结构体
     */
    static WordEntry mapToWord(const QVariantMap &map);


public://临时
    QSqlDatabase database() const { return m_db; }
};

#endif // WORDDATABASEMANAGER_H