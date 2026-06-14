#ifndef WORDFILTERPROXYMODEL_H
#define WORDFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QString>

class WordFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString keyword READ keyword WRITE setKeyword NOTIFY keywordChanged)

public:
    explicit WordFilterProxyModel(QObject *parent = nullptr);

    QString keyword() const;
    void setKeyword(const QString &keyword);

signals:
    void keywordChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_keyword;
};

#endif