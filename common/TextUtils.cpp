// TextUtils.cpp
#include "TextUtils.h"
#include <QChar>
#include <QString>

namespace TextUtils
{

bool isLatinLetter(QChar ch)
{
    const ushort uc = ch.unicode();

    // 基本拉丁字母
    if ((uc >= 'A' && uc <= 'Z') || (uc >= 'a' && uc <= 'z'))
        return true;

    // Latin-1 Supplement 字母区块
    if (uc >= 0x00C0 && uc <= 0x00F6 && uc != 0x00D7)
        return true;
    if (uc >= 0x00F8 && uc <= 0x00FF)
        return true;

    // Latin Extended-A
    if (uc >= 0x0100 && uc <= 0x017F)
        return true;

    // Latin Extended-B
    if (uc >= 0x0180 && uc <= 0x024F)
        return true;

    return false;
}

bool isLetter(QChar ch)
{
    return (ch >= QLatin1Char('a') && ch <= QLatin1Char('z')) ||
           (ch >= QLatin1Char('A') && ch <= QLatin1Char('Z'));
}

bool isAllDigits(const QString &s)
{
    if (s.isEmpty())
        return false;

    for (const QChar &ch : s)
    {
        if (ch.digitValue() == -1)
            return false;
    }
    return true;
}

QString cleanWord(const QString &word)
{
    if (word.isEmpty())
        return QString();

    int start = 0;
    int end = word.length() - 1;

    while (start <= end && word.at(start) == QLatin1Char('-'))
        ++start;
    while (end >= start && word.at(end) == QLatin1Char('-'))
        --end;

    if (start > end)
        return QString();

    return word.mid(start, end - start + 1);
}

QString keepOnlyEnglishLetters(const QString &text)
{
    QString result;
    result.reserve(text.size());

    for (const QChar &ch : text)
    {
        if (isLatinLetter(ch))
        {
            result.append(ch.toLower());
        }
        else if (ch == QLatin1Char('-'))
        {
            result.append(QLatin1Char('-'));
        }
    }

    result.squeeze();
    return result;
}

} // namespace TextUtils