#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QString>
#include <QVariantMap>
#include "WordEntry.h"

class WordListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        WordRole = Qt::UserRole + 1,
        TranslationRole,
        PhoneticRole
    };
    Q_ENUM(Roles)

    explicit WordListModel(QObject *parent = nullptr);

    // ===== 基本接口 =====
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // ===== 高性能接口 =====
    void setDataList(const QVector<WordEntry> &list);

    Q_INVOKABLE void removeAt(int index);
    Q_INVOKABLE QVariantMap get(int index) const;
    Q_INVOKABLE void clear();

    // 🚀 新增：避免 QVariantMap
    Q_INVOKABLE QString getWord(int index) const;
    Q_INVOKABLE QString getTranslation(int index) const;
    Q_INVOKABLE QString getPhonetic(int index) const;

    // 🚀 新增：批量接口
    void appendBatch(const QVector<WordEntry> &list);

private:
    QVector<WordEntry> m_list; // ✅ 连续内存优化
};