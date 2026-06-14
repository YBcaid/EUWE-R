QT += widgets sql axcontainer pdf

CONFIG += c++17
TEMPLATE = app
TARGET = EUWE
QT += widgets sql axcontainer pdf

INCLUDEPATH += ../common ../core ../modules ../ui

SOURCES += main.cpp \
    widget.cpp

# Debug 版本库路径
debug {
    LIBS += -L$$OUT_PWD/../common/debug -lcommon
    LIBS += -L$$OUT_PWD/../core/debug -lcore
    LIBS += -L$$OUT_PWD/../modules/debug -lmodules
    LIBS += -L$$OUT_PWD/../ui/debug -lui
}

# Release 版本库路径
release {
    LIBS += -L$$OUT_PWD/../common/release -lcommon
    LIBS += -L$$OUT_PWD/../core/release -lcore
    LIBS += -L$$OUT_PWD/../modules/release -lmodules
    LIBS += -L$$OUT_PWD/../ui/release -lui
}

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    widget.ui

HEADERS += \
    widget.h
