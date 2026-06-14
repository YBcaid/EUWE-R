#include "SelectionModel.h"
#include <algorithm>

SelectionModel::SelectionModel(QObject *parent)
    : QObject(parent)
{
}

QVector<int> SelectionModel::selected() const
{
    QVector<int> result;
    result.reserve(m_selected.size());
    for (int v : m_selected)
        result.append(v);
    std::sort(result.begin(), result.end());
    return result;
}

int SelectionModel::count() const
{
    return m_selected.size();
}

bool SelectionModel::contains(int index) const
{
    return m_selected.contains(index);
}

int SelectionModel::anchorIndex() const
{
    return m_anchorIndex;
}

void SelectionModel::setAnchorIndex(int index)
{
    if (m_anchorIndex == index)
        return;
    m_anchorIndex = index;
    emit anchorChanged();
}

void SelectionModel::clear()
{
    if (m_selected.isEmpty())
        return;
    m_selected.clear();
    emit selectedChanged();
}

void SelectionModel::toggle(int index)
{
    if (m_selected.contains(index))
        m_selected.remove(index);
    else
        m_selected.insert(index);
    m_anchorIndex = index;
    emit selectedChanged();
    emit anchorChanged();
}

void SelectionModel::selectSingle(int index)
{
    if (m_selected.size() == 1 && m_selected.contains(index))
        return;
    m_selected.clear();
    m_selected.insert(index);
    m_anchorIndex = index;
    emit selectedChanged();
    emit anchorChanged();
}

void SelectionModel::add(int index)
{
    if (m_selected.contains(index))
        return;
    m_selected.insert(index);
    m_anchorIndex = index;
    emit selectedChanged();
    emit anchorChanged();
}

void SelectionModel::remove(int index)
{
    if (!m_selected.contains(index))
        return;
    m_selected.remove(index);
    emit selectedChanged();
}

void SelectionModel::selectRange(int from, int to)
{
    if (from > to)
        std::swap(from, to);
    if (from < 0 || to < 0)
        return;

    m_selected.clear();
    for (int i = from; i <= to; ++i)
        m_selected.insert(i);
    emit selectedChanged();
}

void SelectionModel::addRange(int from, int to)
{
    if (from > to)
        std::swap(from, to);
    if (from < 0 || to < 0)
        return;

    bool changed = false;
    for (int i = from; i <= to; ++i) {
        if (!m_selected.contains(i)) {
            m_selected.insert(i);
            changed = true;
        }
    }
    if (changed)
        emit selectedChanged();
}

void SelectionModel::removeRange(int from, int to)
{
    if (from > to)
        std::swap(from, to);
    if (from < 0 || to < 0)
        return;

    bool changed = false;
    for (int i = from; i <= to; ++i) {
        if (m_selected.contains(i)) {
            m_selected.remove(i);
            changed = true;
        }
    }
    if (changed)
        emit selectedChanged();
}

void SelectionModel::setSelected(const QVector<int> &indices)
{
    QSet<int> newSet;
    for (int idx : indices)
        newSet.insert(idx);
    if (m_selected == newSet)
        return;
    m_selected = newSet;
    emit selectedChanged();
}