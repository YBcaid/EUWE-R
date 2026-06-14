#include "WordManager.h"
#include <QRegularExpression>
#include <QSet>

WordManager::WordManager(QObject *parent)
    : QObject(parent)
{
}

WordListModel* WordManager::newModel()
{
    return &m_newModel;
}

WordListModel* WordManager::knownModel()
{
    return &m_knownModel;
}

WordListModel* WordManager::unknownModel()
{
    return &m_unknownModel;
}

void WordManager::extractText(const QString &text)
{
    m_newModel.clear();
    m_unknownModel.clear();

    QList<WordEntry> result;

    // ===== 词提取（你可以替换成你自己的算法）=====
    QRegularExpression re("\\b[a-zA-Z]+\\b");
    auto it = re.globalMatch(text);

    QSet<QString> unique;

    while (it.hasNext()) {
        auto match = it.next();
        QString word = match.captured().toLower();

        if (unique.contains(word)) continue;
        unique.insert(word);

        WordEntry e;
        e.word = word;
        e.translation = "";
        e.phonetic = "";

        result.append(e);
    }

    m_newModel.setDataList(result);
}