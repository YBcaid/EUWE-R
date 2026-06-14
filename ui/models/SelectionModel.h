#ifndef SELECTIONMODEL_H
#define SELECTIONMODEL_H

#include <QObject>
#include <QSet>
#include <QVector>

class SelectionModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVector<int> selected READ selected NOTIFY selectedChanged)
    Q_PROPERTY(int count READ count NOTIFY selectedChanged)
    Q_PROPERTY(int anchorIndex READ anchorIndex WRITE setAnchorIndex NOTIFY anchorChanged)

public:
    explicit SelectionModel(QObject *parent = nullptr);

    QVector<int> selected() const;
    int count() const;

    // ✅ 必须标记 Q_INVOKABLE，否则 QML 无法调用
    Q_INVOKABLE bool contains(int index) const;

    int anchorIndex() const;
    void setAnchorIndex(int index);

    Q_INVOKABLE void clear();
    Q_INVOKABLE void toggle(int index);
    Q_INVOKABLE void selectSingle(int index);
    Q_INVOKABLE void add(int index);
    Q_INVOKABLE void remove(int index);

    Q_INVOKABLE void selectRange(int from, int to);
    Q_INVOKABLE void addRange(int from, int to);
    Q_INVOKABLE void removeRange(int from, int to);
    Q_INVOKABLE void setSelected(const QVector<int> &indices);

signals:
    void selectedChanged();
    void anchorChanged();

private:
    QSet<int> m_selected;
    int m_anchorIndex = -1;
};

#endif // SELECTIONMODEL_H