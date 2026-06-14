#include "WordFilterProxyModel.h"
#include "WordListModel.h"

WordFilterProxyModel::WordFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

QString WordFilterProxyModel::keyword() const
{
    return m_keyword;
}

void WordFilterProxyModel::setKeyword(const QString &keyword)
{
    QString k = keyword.trimmed();

    if (m_keyword == k)
        return;

    m_keyword = k;
    invalidateFilter();

    emit keywordChanged();
}

bool WordFilterProxyModel::filterAcceptsRow(int sourceRow,
                                            const QModelIndex &sourceParent) const
{
    if (m_keyword.isEmpty())
        return true;

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    QString word = sourceModel()->data(index, WordListModel::WordRole).toString();
    QString translation = sourceModel()->data(index, WordListModel::TranslationRole).toString();
    QString phonetic = sourceModel()->data(index, WordListModel::PhoneticRole).toString();

    return word.contains(m_keyword, Qt::CaseInsensitive)
           || translation.contains(m_keyword, Qt::CaseInsensitive)
           || phonetic.contains(m_keyword, Qt::CaseInsensitive);
}