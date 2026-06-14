TEMPLATE = lib
CONFIG += staticlib
TARGET = ui
QT += widgets sql axcontainer pdf
QT += core gui widgets quick quickcontrols2 sql
TARGET = ui
CONFIG += c++17


QT += quick               # QML 引擎核心
QT += quickcontrols2      # 注意去掉横杠（之前写了 quick-controls2）
QT += quicklayouts       # 这个是对的（有横杠）
QT += quickwidgets        # 注意去掉横杠（之前写了 quick-widgets）


INCLUDEPATH += ../common ../core ../modules

HEADERS += \
    main/MainController.h \
    models/SelectionModel.h \
    result/ResultDisplayWidget.h \
    database/DatabaseManagementWindow.h \
    models/WordListModel.h \
    models/WordManager.h \
    models/WordFilterProxyModel.h

SOURCES += \
    main/MainController.cpp \
    models/SelectionModel.cpp \
    result/ResultDisplayWidget.cpp \
    database/DatabaseManagementWindow.cpp \
    models/WordListModel.cpp \
    models/WordManager.cpp \
    models/WordFilterProxyModel.cpp

RESOURCES +=

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    main/CapsuleButton.qml \
    main/ExtractPage.qml \
    main/MessageDialog.qml \
    main/PanelGroup.qml \
    main/ResultPage.qml \
    main/SelectableWordList.qml \
    main/SideBarItem.qml \
    main/Theme.qml \
    main/ThemeToggleBtn.qml \
    main/WindowControlBtn.qml \
    main/WordManagePage.qml \
    main/WordTable.qml \
    main/main.qml \
    main/qmldir \
    main/rewrite/ColumnLayoutModel.qml \
    main/rewrite/SelectableTable.qml \
    main/rewrite/SelectionController.qml \
    main/rewrite/TableHeader.qml \
    main/rewrite/TableRow.qml \
    main/rewrite/WordPage.qml \
    main/rewrite/main.qml \
    result/ResultDisplayWidget.qml