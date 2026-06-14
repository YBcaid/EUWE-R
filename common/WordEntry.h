#ifndef WordEntry_H
#define WordEntry_H

#include <QString>

/**
 * @brief 单词结构体
 * 包含单词原文、翻译及音标信息，用于存储单个单词的完整释义。
 */
struct WordEntry
{
    /**
     * @brief 构造一个单词条目
     * @param WordEntry 单词原文
     * @param translation 单词的中文翻译
     * @param phonetic 音标（如 IPA / DJ 音标）
     */
    explicit WordEntry(const QString &word = QString(), const QString &translation = QString(),
                  const QString &phonetic = QString())
        : word(word)
        , translation(translation)
        , phonetic(phonetic)
    {
    }

    QString word;		 /**< 单词原文 */
    QString translation; /**< 中文翻译 */
    QString phonetic;	 /**< 音标 */
};

#endif // WordEntry_H