// WordExtractor.h
#ifndef WORDEXTRACTOR_H
#define WORDEXTRACTOR_H

#include <QObject>
#include <QString>
#include <unordered_set>

class WordDatabaseManager;

/**
 * @brief 英文（拉丁字母）单词提取器
 *
 * 从给定文本中提取单词，支持跨行连字符断字合并，通过词典验证合并结果。
 * 提取过程仅保留拉丁字母（含扩展）和连字符，单词统一小写，并过滤纯数字串。
 *
 * 线程安全性：应在与 WordDatabaseManager 相同的线程中使用。
 */
class WordExtractor : public QObject
{
    Q_OBJECT

    public:
    /**
     * @brief 构造函数
     * @param text 待提取的文本（可选，构造时自动执行提取）
     * @param dbManager 单词数据库管理器指针，用于验证单词存在性（可为 nullptr）
     * @param parent Qt 父对象
     */
    explicit WordExtractor(const QString &text = QString(),
                           WordDatabaseManager *dbManager = nullptr, QObject *parent = nullptr);

    /**
     * @brief 执行提取操作，结果存储在内部集合中
     * @param text 待提取的文本
     */
    void extract(const QString &text);

    /**
     * @brief 获取提取出的单词集合（移动语义）
     * @return 单词集合；每次调用后内部集合被清空
     */
    std::unordered_set<QString> getWords();

    private:
    /**
     * @brief 判断单词是否在词典中存在
     * @param word 待查询的单词（已规范化小写）
     * @return 存在返回 true；dbManager 为空或查询失败返回 false
     */
    bool isWordInDictionary(const QString &word) const;

    /**
     * @brief 决策跨行合并策略
     * @param part1 换行前的单词部分（可能包含内部连字符）
     * @param part2 换行后的字母序列（可能带前导连字符）
     * @param hasHyphenBeforeNewline 换行前是否有一个未决的连字符
     * @return 合并后的单词字符串；若不合并返回空字符串
     */
    QString tryMergeAcrossNewline(const QString &part1, const QString &part2,
                                  bool hasHyphenBeforeNewline) const;

    std::unordered_set<QString> m_words; ///< 已提取的单词集合
    WordDatabaseManager *m_dbManager;	 ///< 词典管理器（可能为空）
};

#endif // WORDEXTRACTOR_H