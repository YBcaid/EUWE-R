#include "WordListModel.h"

WordListModel::WordListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int WordListModel::rowCount(const QModelIndex &) const
{
    return m_list.size();
}

QVariant WordListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    const WordEntry &w = m_list.at(index.row());

    switch (role)
    {
    case WordRole:
        return w.word;
    case TranslationRole:
        return w.translation;
    case PhoneticRole:
        return w.phonetic;
    default:
        return {};
    }
}

QHash<int, QByteArray> WordListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        {WordRole, "word"},
        {TranslationRole, "translation"},
        {PhoneticRole, "phonetic"}
    };
    return roles;
}

void WordListModel::setDataList(const QVector<WordEntry> &list)
{
    beginResetModel();
    m_list = list;
    endResetModel();
}

void WordListModel::appendBatch(const QVector<WordEntry> &list)
{
    if (list.isEmpty())
        return;

    int start = m_list.size();
    int end = start + list.size() - 1;

    beginInsertRows(QModelIndex(), start, end);
    m_list += list;
    endInsertRows();
}

void WordListModel::removeAt(int index)
{
    if (index < 0 || index >= m_list.size())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    m_list.removeAt(index);
    endRemoveRows();
}

void WordListModel::clear()
{
    beginResetModel();
    m_list.clear();
    endResetModel();
}

QVariantMap WordListModel::get(int index) const
{
    if (index < 0 || index >= m_list.size())
        return {};

    const WordEntry &w = m_list.at(index);

    return {
        {"word", w.word},
        {"translation", w.translation},
        {"phonetic", w.phonetic}
    };
}

QString WordListModel::getWord(int index) const
{
    if (index < 0 || index >= m_list.size())
        return {};
    return m_list.at(index).word;
}

QString WordListModel::getTranslation(int index) const
{
    if (index < 0 || index >= m_list.size())
        return {};
    return m_list.at(index).translation;
}

QString WordListModel::getPhonetic(int index) const
{
    if (index < 0 || index >= m_list.size())
        return {};
    return m_list.at(index).phonetic;
}