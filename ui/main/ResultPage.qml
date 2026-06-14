import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "transparent"

    property ListModel unknownModel
    property ListModel newWordModel
    property ListModel knownWordModel

    property var unknownSelected: []
    property var newWordSelected: []
    property var knownWordSelected: []
    property string activeList: ""
    property var activeSelected: []

    function clearAllSelections() {
        unknownListComp.clearSelection()
        newWordListComp.clearSelection()
        knownWordListComp.clearSelection()
        activeList = ""; activeSelected = []
    }

    function clearOtherList(listName) {
        if (listName !== "unknown") unknownListComp.clearSelection()
        if (listName !== "newWord") newWordListComp.clearSelection()
        if (listName !== "knownWord") knownWordListComp.clearSelection()
    }

    function colorToHex(color) {
        var str = color.toString()
        if (str.startsWith("#")) return str
        var match = str.match(/rgba?\((\d+),\s*(\d+),\s*(\d+)/)
        if (match) {
            var r = parseInt(match[1]).toString(16).padStart(2, '0')
            var g = parseInt(match[2]).toString(16).padStart(2, '0')
            var b = parseInt(match[3]).toString(16).padStart(2, '0')
            return "#" + r + g + b
        }
        return "#000000"
    }

    readonly property string svgUnknown: "data:image/svg+xml;utf8,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'%3E%3Ccircle cx='12' cy='12' r='10'/%3E%3Cpath d='M10 9a2 2 0 0 1 4 0c0 2-2 3-2 5'/%3E%3Ccircle cx='12' cy='17' r='1' fill='currentColor' stroke='none'/%3E%3C/svg%3E"
    readonly property string svgNewWord: "data:image/svg+xml;utf8,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'%3E%3Cpath d='M4 6h16v12H4z'/%3E%3Cpath d='M4 6v12'/%3E%3Cpath d='M20 6v12'/%3E%3Cpath d='M12 6v12'/%3E%3C/svg%3E"
    readonly property string svgKnown: "data:image/svg+xml;utf8,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'%3E%3Ccircle cx='12' cy='12' r='10'/%3E%3Cpolyline points='8 12 11 15 16 9'/%3E%3C/svg%3E"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        Text {
            text: "📊 单词识别结果"
            font.pixelSize: 18
            font.bold: true
            color: Theme.textDark
            Layout.alignment: Qt.AlignHCenter
        }

        SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal

            handle: Rectangle {
                implicitWidth: 2
                color: handleMouse.containsMouse ? Theme.accent : "transparent"
                Behavior on color { ColorAnimation { duration: 200 } }
                MouseArea { id: handleMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.SplitHCursor }
            }

            // 未识别 → 使用 SelectableWordList
            Rectangle {
                SplitView.preferredWidth: 140; SplitView.minimumWidth: 100
                border.color: Qt.rgba(Theme.borderInner.r, Theme.borderInner.g, Theme.borderInner.b, 0.5)
                border.width: 1; radius: 14; color: Theme.surface
                Rectangle { anchors.fill: parent; radius: 14; color: Theme.dark ? "transparent" : Qt.rgba(1,1,1,0.4) }

                SelectableWordList {
                    id: unknownListComp
                    anchors.fill: parent
                    anchors.margins: 12
                    title: "未识别"
                    iconColor: colorToHex(Theme.btnDangerText)
                    iconSvg: svgUnknown
                    wordModel: unknownModel
                    activeListType: "unknown"
                    showTranslation: false
                    showPhonetic: false

                    onSelectionChanged: function(indices) {
                        unknownSelected = indices.slice()
                        root.activeList = "unknown"
                        root.activeSelected = unknownSelected
                    }
                    onClearOtherLists: function(activeType) { root.clearOtherList(activeType) }
                }
            }

            // 生词列表
            Rectangle {
                SplitView.preferredWidth: 400; SplitView.minimumWidth: 250
                border.color: Qt.rgba(Theme.borderInner.r, Theme.borderInner.g, Theme.borderInner.b, 0.5)
                border.width: 1; radius: 14; color: Theme.surface
                Rectangle { anchors.fill: parent; radius: 14; color: Theme.dark ? "transparent" : Qt.rgba(1,1,1,0.4) }
                SelectableWordList {
                    id: newWordListComp
                    anchors.fill: parent; anchors.margins: 12
                    title: "生词（已获取释义）"
                    iconColor: Theme.accent
                    iconSvg: svgNewWord
                    wordModel: newWordModel
                    activeListType: "newWord"
                    showTranslation: true; showPhonetic: true
                    onSelectionChanged: function(indices) {
                        newWordSelected = indices.slice()
                        root.activeList = "newWord"
                        root.activeSelected = newWordSelected
                    }
                    onClearOtherLists: function(activeType) { root.clearOtherList(activeType) }
                }
            }

            // 熟词列表
            Rectangle {
                SplitView.preferredWidth: 140; SplitView.minimumWidth: 100
                border.color: Qt.rgba(Theme.borderInner.r, Theme.borderInner.g, Theme.borderInner.b, 0.5)
                border.width: 1; radius: 14; color: Theme.surface
                Rectangle { anchors.fill: parent; radius: 14; color: Theme.dark ? "transparent" : Qt.rgba(1,1,1,0.4) }
                SelectableWordList {
                    id: knownWordListComp
                    anchors.fill: parent; anchors.margins: 12
                    title: "熟词"
                    iconColor: "#28a745"
                    iconSvg: svgKnown
                    wordModel: knownWordModel
                    activeListType: "knownWord"
                    showTranslation: false; showPhonetic: false
                    onSelectionChanged: function(indices) {
                        knownWordSelected = indices.slice()
                        root.activeList = "knownWord"
                        root.activeSelected = knownWordSelected
                    }
                    onClearOtherLists: function(activeType) { root.clearOtherList(activeType) }
                }
            }
        }

        // 底部按钮栏
        RowLayout {
            Layout.fillWidth: true; spacing: 12
            Item {
                id: exportBtnWrapper; Layout.preferredWidth: 100; Layout.preferredHeight: 34
                CapsuleButton { anchors.fill: parent; text: "导出 ▴"; iconType: "upload"; isPrimary: false
                    onClicked: {
                        if (exportPopup.visible) { exportPopup.close() }
                        else {
                            var btnPos = exportBtnWrapper.mapToItem(root, 0, 0)
                            var popupWidth = 180; var popupHeight = 132
                            var popupY = btnPos.y - popupHeight - 4
                            exportPopup.isUpward = true
                            if (popupY < 0) { popupY = btnPos.y + exportBtnWrapper.height + 4; exportPopup.isUpward = false }
                            var popupX = btnPos.x
                            if (popupX + popupWidth > root.width) popupX = root.width - popupWidth
                            if (popupX < 0) popupX = 0
                            exportPopup.x = popupX; exportPopup.y = popupY; exportPopup.open()
                        }
                    }
                }
            }

            Item {
                Layout.preferredWidth: 120; Layout.preferredHeight: 34
                CapsuleButton { anchors.fill: parent
                    text: {
                        if (activeList === "newWord" && newWordSelected.length > 0) return "录入熟词库(" + newWordSelected.length + ")"
                        if (activeList === "knownWord" && knownWordSelected.length > 0) return "移出熟词库(" + knownWordSelected.length + ")"
                        return "熟词库操作"
                    }
                    iconType: ""; isPrimary: false
                    onClicked: {
                        if (activeList === "newWord" && newWordSelected.length > 0) {
                            var indices = newWordSelected.slice().sort((a,b) => b - a)
                            for (var i = 0; i < indices.length; i++) {
                                var idx = indices[i]
                                var item = newWordModel.get(idx)
                                knownWordModel.append({ word: item.word, translation: item.translation, phonetic: item.phonetic })
                                newWordModel.remove(idx)
                            }
                            clearAllSelections()
                            if (root.mainWindow) root.mainWindow.statusTextValue = `✅ 已录入 ${indices.length} 个单词到熟词库`
                        } else if (activeList === "knownWord" && knownWordSelected.length > 0) {
                            var indices = knownWordSelected.slice().sort((a,b) => b - a)
                            for (var j = 0; j < indices.length; j++) {
                                var idx = indices[j]
                                var item = knownWordModel.get(idx)
                                newWordModel.append({ word: item.word, translation: item.translation, phonetic: item.phonetic })
                                knownWordModel.remove(idx)
                            }
                            clearAllSelections()
                            if (root.mainWindow) root.mainWindow.statusTextValue = `↩️ 已移出 ${indices.length} 个单词到生词`
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true }
            Item { Layout.preferredWidth: 120; Layout.preferredHeight: 34
                CapsuleButton { anchors.fill: parent; text: "数据库管理"; isPrimary: false
                    onClicked: { if (root.mainWindow) root.mainWindow.currentView = 2 } }
            }
            Item { Layout.preferredWidth: 100; Layout.preferredHeight: 34
                CapsuleButton { anchors.fill: parent; text: "继续识别"; isPrimary: true
                    onClicked: { if (root.mainWindow) root.mainWindow.currentView = 0 } }
            }
        }
    }

    // 导出菜单（未改动）
    Popup {
        id: exportPopup; padding: 0; closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnEscape; focus: true
        property bool isUpward: false
        background: Item {
            implicitWidth: 180; implicitHeight: colLayout.implicitHeight + 12
            Rectangle { x: 6; y: 8; width: parent.width; height: parent.height; radius: 14; color: Qt.rgba(0,0,0,0.03) }
            Rectangle { x: 3; y: 4; width: parent.width; height: parent.height; radius: 14; color: Qt.rgba(0,0,0,0.06) }
            Rectangle { x: 1; y: 2; width: parent.width; height: parent.height; radius: 14; color: Qt.rgba(0,0,0,0.10) }
            Rectangle { anchors.fill: parent; radius: 14
                color: Theme.dark ? Qt.lighter(Theme.surface, 1.08) : Qt.darker(Theme.surface, 1.01)
                border.color: Theme.dark ? Theme.accent : Qt.rgba(Theme.borderInner.r, Theme.borderInner.g, Theme.borderInner.b, 0.5)
                border.width: Theme.dark ? 1.5 : 1.2; clip: true }
        }
        contentItem: Item {
            anchors.fill: parent; anchors.margins: 1
            Column { id: colLayout; anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top; anchors.margins: 6; spacing: 0
                Repeater {
                    model: [
                        { text: "CSV 导出", icon: "data:image/svg+xml;utf8,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'%3E%3Crect x='3' y='3' width='18' height='18' rx='2'/%3E%3Cline x1='8' y1='10' x2='16' y2='10'/%3E%3Cline x1='8' y1='14' x2='16' y2='14'/%3E%3Cline x1='8' y1='6' x2='16' y2='6'/%3E%3C/svg%3E" },
                        { text: "PDF 导出", icon: "data:image/svg+xml;utf8,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'%3E%3Cpath d='M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z'/%3E%3Cpolyline points='14 2 14 8 20 8'/%3E%3Ctext x='8' y='17' font-size='5' font-weight='bold' fill='currentColor' stroke='none'%3EPDF%3C/text%3E%3C/svg%3E" },
                        { text: "DOC 导出", icon: "data:image/svg+xml;utf8,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'%3E%3Cpath d='M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z'/%3E%3Cpolyline points='14 2 14 8 20 8'/%3E%3Ctext x='7' y='17' font-size='4.5' font-weight='bold' fill='currentColor' stroke='none'%3EDOC%3C/text%3E%3C/svg%3E" }
                    ]
                    delegate: Rectangle {
                        width: colLayout.width; height: 40; radius: 8
                        color: hoverArea.containsMouse ? Theme.ghostBtnHoverBg : "transparent"
                        Image { id: iconImg; width: 16; height: 16; anchors.left: parent.left; anchors.leftMargin: 12; anchors.verticalCenter: parent.verticalCenter
                            source: modelData.icon.replace("currentColor", Theme.dark ? "%23FFFFFF" : "%234B5563"); sourceSize: Qt.size(16,16); smooth: true }
                        Text { anchors.left: iconImg.right; anchors.leftMargin: 8; anchors.verticalCenter: parent.verticalCenter
                            text: modelData.text; font.pixelSize: 13; font.weight: Font.Medium; color: Theme.textDark }
                        Rectangle { anchors.bottom: parent.bottom; width: parent.width-24; height: 1
                            color: Theme.dark ? Qt.lighter(Theme.borderInner, 1.3) : Theme.borderInner; visible: index !== 2; anchors.horizontalCenter: parent.horizontalCenter }
                        MouseArea { id: hoverArea; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                            onClicked: { console.log(modelData.text); exportPopup.close() } }
                    }
                }
            }
        }
        enter: Transition { ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
            NumberAnimation { property: "y"; from: exportPopup.isUpward ? exportPopup.y + 20 : exportPopup.y - 6; to: exportPopup.y; duration: 260; easing.type: Easing.OutBack } } }
        exit: Transition { ParallelAnimation {
            NumberAnimation { property: "opacity"; to: 0; duration: 140 }
            NumberAnimation { property: "y"; to: exportPopup.isUpward ? exportPopup.y + 12 : exportPopup.y + 10; duration: 180; easing.type: Easing.InCubic } } }
    }

    property var mainWindow: null
}