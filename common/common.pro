TEMPLATE = lib
CONFIG += staticlib
TARGET = common
CONFIG += c++17

HEADERS += \
    TextUtils.h \
    AppConfig.h \
    Logger.h \
    WordEntry.h \
    common.h

SOURCES += \
    TextUtils.cpp \
    AppConfig.cpp \
    Logger.cpp \
    common.cpp

unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target