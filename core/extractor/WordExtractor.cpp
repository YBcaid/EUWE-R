// WordExtractor.cpp
#include "WordExtractor.h"
#include "../database/WordDatabaseManager.h"
#include "TextUtils.h"
#include <QDebug>

// ---------- 构造 ----------

WordExtractor::WordExtractor(const QString &text, WordDatabaseManager *dbManager, QObject *parent)
    : QObject(parent)
    , m_dbManager(dbManager)
{
    if (!text.isEmpty())
    {
        extract(text);
    }
}

// ---------- 公共接口 ----------

void WordExtractor::extract(const QString &text)
{
    m_words.clear();

    if (text.isEmpty())
    {
        return;
    }

    QString current;				// 当前正在构建的单词片段（小写字母 + 可能的内置连字符）
    bool hasTrailingHyphen = false; // current 末尾是否有一个未决的连字符
    bool afterNewline = false;		// 上一个有效字符是换行符，且 current 非空
    int i = 0;

    // 末尾添加空格作为结束标志，简化边界处理
    QString workingText = text;
    workingText += QLatin1Char(' ');
    const int len = workingText.length();

    while (i < len)
    {
        const QChar c = workingText.at(i);

        // ---------- 字母 ----------
        if (TextUtils::isLatinLetter(c))
        {
            if (afterNewline && !current.isEmpty())
            {
                // 单词被换行打断，前瞻收集换行后紧随的字母序列（可能包含前导连字符）
                int j = i;
                QString following;

                // 收集可能的前导连字符
                while (j < len && workingText.at(j) == QLatin1Char('-'))
                {
                    following += QLatin1Char('-');
                    ++j;
                }
                // 收集连续的字母（统一小写）
                while (j < len && TextUtils::isLatinLetter(workingText.at(j)))
                {
                    following += workingText.at(j).toLower();
                    ++j;
                }

                if (!following.isEmpty())
                {
                    const QString merged =
                        tryMergeAcrossNewline(current, following, hasTrailingHyphen);
                    if (!merged.isEmpty())
                    {
                        current = merged;
                    }
                    else
                    {
                        // 不合并：保存 current，开始新单词 following
                        const QString cleanedCurrent = TextUtils::cleanWord(current);
                        if (!cleanedCurrent.isEmpty() && !TextUtils::isAllDigits(cleanedCurrent))
                        {
                            m_words.insert(cleanedCurrent);
                        }
                        current = following; // following 已为小写
                    }
                }

                i = j;
                afterNewline = false;
                hasTrailingHyphen = false;
                continue;
            }

            // 常规字母追加（统一小写）
            if (hasTrailingHyphen)
            {
                hasTrailingHyphen = false;
            }
            current += c.toLower();
            afterNewline = false;
            ++i;
        }
        // ---------- 连字符 '-' ----------
        else if (c == QLatin1Char('-'))
        {
            if (afterNewline && !current.isEmpty())
            {
                int j = i;
                QString following;

                while (j < len && workingText.at(j) == QLatin1Char('-'))
                {
                    following += QLatin1Char('-');
                    ++j;
                }
                while (j < len && TextUtils::isLatinLetter(workingText.at(j)))
                {
                    following += workingText.at(j).toLower();
                    ++j;
                }

                if (!following.isEmpty() && following.at(0) == QLatin1Char('-'))
                {
                    const QString merged =
                        tryMergeAcrossNewline(current, following, hasTrailingHyphen);
                    if (!merged.isEmpty())
                    {
                        current = merged;
                    }
                    else
                    {
                        const QString cleanedCurrent = TextUtils::cleanWord(current);
                        if (!cleanedCurrent.isEmpty() && !TextUtils::isAllDigits(cleanedCurrent))
                        {
                            m_words.insert(cleanedCurrent);
                        }
                        current = following;
                    }
                    i = j;
                    afterNewline = false;
                    hasTrailingHyphen = false;
                    continue;
                }
                else
                {
                    // 连字符后无字母，结束 current
                    const QString cleaned = TextUtils::cleanWord(current);
                    if (!cleaned.isEmpty() && !TextUtils::isAllDigits(cleaned))
                    {
                        m_words.insert(cleaned);
                    }
                    current.clear();
                    hasTrailingHyphen = false;
                    afterNewline = false;
                    ++i;
                    continue;
                }
            }

            // 非换行上下文下的连字符处理
            if (!current.isEmpty() && current.at(current.length() - 1) != QLatin1Char('-'))
            {
                hasTrailingHyphen = true;
                current += QLatin1Char('-');
                ++i;
            }
            else if (current.isEmpty())
            {
                current += QLatin1Char('-');
                ++i;
            }
            else
            {
                // 连续连字符 "a--b"，结束当前单词，新单词以 '-' 开始
                const QString cleaned = TextUtils::cleanWord(current);
                if (!cleaned.isEmpty() && !TextUtils::isAllDigits(cleaned))
                {
                    m_words.insert(cleaned);
                }
                current = QStringLiteral("-");
                hasTrailingHyphen = true;
                ++i;
            }
            afterNewline = false;
        }
        // ---------- 换行符 '\n' ----------
        else if (c == QLatin1Char('\n'))
        {
            if (!current.isEmpty())
            {
                afterNewline = true;
            }
            ++i;
        }
        // ---------- 其他分隔符 ----------
        else
        {
            if (!current.isEmpty())
            {
                const QString cleaned = TextUtils::cleanWord(current);
                if (!cleaned.isEmpty() && !TextUtils::isAllDigits(cleaned))
                {
                    m_words.insert(cleaned);
                }
                current.clear();
                hasTrailingHyphen = false;
            }
            afterNewline = false;
            ++i;
        }
    }

    // 处理末尾可能残留的单词
    if (!current.isEmpty())
    {
        const QString cleaned = TextUtils::cleanWord(current);
        if (!cleaned.isEmpty() && !TextUtils::isAllDigits(cleaned))
        {
            m_words.insert(cleaned);
        }
    }
}

std::unordered_set<QString> WordExtractor::getWords()
{
    for (const auto &word : m_words)
    {
        qDebug() << word;
    }
    return std::move(m_words);
}

// ---------- 私有辅助方法 ----------

bool WordExtractor::isWordInDictionary(const QString &word) const
{
    if (!m_dbManager)
    {
        qWarning() << "WordDatabaseManager not set, dictionary lookup disabled.";
        return false;
    }
    return m_dbManager->containsWord(word);
}

QString WordExtractor::tryMergeAcrossNewline(const QString &part1, const QString &part2,
                                             bool hasHyphenBeforeNewline) const
{
    QString part2Clean = part2;
    bool part2StartsWithHyphen = false;
    while (!part2Clean.isEmpty() && part2Clean.at(0) == QLatin1Char('-'))
    {
        part2Clean.remove(0, 1);
        part2StartsWithHyphen = true;
    }

    if (part2Clean.isEmpty())
    {
        return QString();
    }

    const QString candidateWithHyphen = part1 + QLatin1Char('-') + part2Clean;
    const QString candidateWithoutHyphen = part1 + part2Clean;

    if (isWordInDictionary(candidateWithoutHyphen))
    {
        return candidateWithoutHyphen;
    }
    if (isWordInDictionary(candidateWithHyphen))
    {
        return candidateWithHyphen;
    }

    if (hasHyphenBeforeNewline || part2StartsWithHyphen)
    {
        return candidateWithHyphen;
    }

    if (TextUtils::isAllDigits(part1))
    {
        return candidateWithHyphen;
    }

    return QString();
}