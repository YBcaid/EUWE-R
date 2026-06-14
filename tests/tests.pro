QT += widgets
CONFIG += c++17
TEMPLATE = app
QT += testlib sql
TARGET = EUWE_tests

QT += core gui widgets quick quickcontrols2 sql
QT += widgets quick quickcontrols2 testlib sql pdf printsupport

INCLUDEPATH += ../common ../core ../modules ../ui

SOURCES += \
    DatabaseManagementWindow.cpp \
    DebugTool.cpp \
    DebugWindow.cpp \
    main.cpp \
    widget.cpp

HEADERS += \
    DatabaseManagementWindow.h \
    DebugTool.h \
    DebugWindow.h \
    widget.h

debug {
    LIBS += -L$$OUT_PWD/../ui/debug -lui
    LIBS += -L$$OUT_PWD/../modules/debug -lmodules
    LIBS += -L$$OUT_PWD/../core/debug -lcore
    LIBS += -L$$OUT_PWD/../common/debug -lcommon
}
release {
    LIBS += -L$$OUT_PWD/../ui/release -lui
    LIBS += -L$$OUT_PWD/../modules/release -lmodules
    LIBS += -L$$OUT_PWD/../core/release -lcore
    LIBS += -L$$OUT_PWD/../common/release -lcommon
}

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    DatabaseManagementWindow.ui \
    DebugWindow.ui \
    widget.ui

DEFINES += PROJECT_ROOT=\\\"$$PWD/..\\\"