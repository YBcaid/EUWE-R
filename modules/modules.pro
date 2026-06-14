TEMPLATE = lib
CONFIG += staticlib


QT += core gui pdf core-private
CONFIG += c++17
CONFIG += qt

TARGET = modules

INCLUDEPATH += ../common ../core

SOURCES += \
    ai/WordCorrection.cpp \
    export/ExportManager.cpp \
    import/FileImporter.cpp \
    modules.cpp \
    ocr/OcrManager.cpp \
    server/LocalServer.cpp \
    updater/UpdateChecker.cpp

HEADERS += \
    ai/WordCorrection.h \
    export/ExportManager.h \
    import/FileImporter.h \
    modules.h \
    ocr/OcrManager.h \
    server/LocalServer.h \
    updater/UpdateChecker.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target
