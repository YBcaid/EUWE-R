#pragma once

#include <QObject>
#include "models/WordListModel.h"
#include "WordEntry.h"

class WordManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(WordListModel* newModel READ newModel CONSTANT)
    Q_PROPERTY(WordListModel* knownModel READ knownModel CONSTANT)
    Q_PROPERTY(WordListModel* unknownModel READ unknownModel CONSTANT)

public:
    explicit WordManager(QObject *parent = nullptr);

    WordListModel* newModel();
    WordListModel* knownModel();
    WordListModel* unknownModel();

    Q_INVOKABLE void extractText(const QString &text);

private:
    WordListModel m_newModel;
    WordListModel m_knownModel;
    WordListModel m_unknownModel;
};