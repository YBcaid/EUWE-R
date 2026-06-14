TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++17
TARGET = core
QT += sql

INCLUDEPATH += ../common

SOURCES += \
    core.cpp \
    database/WordDatabaseManager.cpp \
    database/DatabaseTableManager.cpp \
    dictionary/LookupService.cpp \
    extractor/WordExtractor.cpp \
    learning/SpacedRepetition.cpp \
    statistics/WordStatistician.cpp

HEADERS += \
    core.h \
    database/WordDatabaseManager.h \
    database/DatabaseTableManager.h \
    dictionary/LookupService.h \
    extractor/WordExtractor.h \
    learning/SpacedRepetition.h \
    statistics/WordStatistician.h

unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target
