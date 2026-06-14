// ui/main/SelectableWordList.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import App.Models 1.0

Item {
    id: root

    property string title: "列表"
    property string iconColor: "#000000"
    property string iconSvg: ""
    property ListModel wordModel
    property bool showTranslation: false
    property bool showPhonetic: false
    property string activeListType: ""

    property SelectionModel selection: SelectionModel {}

    signal clearOtherLists(string activeType)

    // 让组件可获得焦点以响应键盘快捷键
    focus: true
    activeFocusOnTab: true

    // 全选
    function selectAll() {
        if (!wordModel) return
        var arr = []
        for (var i = 0; i < wordModel.count; ++i)
            arr.push(i)
        selection.setSelected(arr)
    }

    // 反选
    function invertSelection() {
        if (!wordModel) return
        var current = selection.selected
        var newArr = []
        for (var i = 0; i < wordModel.count; ++i) {
            if (current.indexOf(i) === -1)
                newArr.push(i)
        }
        selection.setSelected(newArr)
    }

    function clearSelection() {
        selection.clear()
    }

    function isSelected(index) {
        return selection.contains(index)
    }

    Keys.onPressed: function(event) {
        // Ctrl+A 全选
        if (event.key === Qt.Key_A && event.modifiers & Qt.ControlModifier) {
            selectAll()
            event.accepted = true
        }
        // Ctrl+C 反选
        else if (event.key === Qt.Key_C && event.modifiers & Qt.ControlModifier) {
            invertSelection()
            event.accepted = true
        }
    }

    function clearOtherList(listType) {
        clearOtherLists(listType)
    }

    // 拖拽选中状态，用于在拖拽时禁用列表自身的拖拽滚动
    QtObject {
        id: dragState
        property bool active: false
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        // 标题行
        Row {
            spacing: 6
            Image {
                width: 16; height: 16; anchors.verticalCenter: parent.verticalCenter
                source: iconSvg ? iconSvg.replace("currentColor", "%23" + iconColor.substring(1)) : ""
                sourceSize: Qt.size(16,16); smooth: true
            }
            Text {
                text: title
                font.pixelSize: 14; font.bold: true
                color: iconColor
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"

            // 主列表视图
            ListView {
                id: listView
                anchors.fill: parent
                model: wordModel
                clip: true
                cacheBuffer: 200
                boundsBehavior: Flickable.StopAtBounds
                interactive: !dragState.active
                highlightFollowsCurrentItem: false
                currentIndex: -1

                // 表头
                header: RowLayout {
                    width: listView.width
                    height: 30
                    spacing: 0

                    Rectangle {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.preferredWidth: listView.width * 0.45
                        radius: 6
                        color: Theme.dark ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.03)
                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 14
                            anchors.verticalCenter: parent.verticalCenter
                            text: "单词"; font.pixelSize: 12; font.bold: true
                            color: Theme.textMid
                        }
                    }

                    Rectangle {
                        Layout.fillHeight: true
                        Layout.fillWidth: showTranslation
                        Layout.preferredWidth: showTranslation ? listView.width * 0.3 : 0
                        radius: 6; visible: showTranslation
                        color: Theme.dark ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.03)
                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 14
                            anchors.verticalCenter: parent.verticalCenter
                            text: "释义"; font.pixelSize: 12; font.bold: true
                            color: Theme.textMid
                        }
                    }

                    Rectangle {
                        Layout.fillHeight: true
                        Layout.fillWidth: showPhonetic
                        Layout.preferredWidth: showPhonetic ? listView.width * 0.25 : 0
                        radius: 6; visible: showPhonetic
                        color: Theme.dark ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.03)
                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 14
                            anchors.verticalCenter: parent.verticalCenter
                            text: "音标"; font.pixelSize: 12; font.bold: true
                            color: Theme.textMid
                        }
                    }
                }

                // 数据行
                delegate: Rectangle {
                    width: listView.width
                    height: 44
                    radius: 6
                    color: "transparent"

                    property var m: model
                    property bool selected: selection.selected.indexOf(index) !== -1

                    // 选中背景
                    Rectangle {
                        anchors.fill: parent; radius: 6
                        color: Theme.accent
                        opacity: selected ? (Theme.dark ? 0.12 : 0.18) : 0.0
                        Behavior on opacity {
                            NumberAnimation { duration: 150; easing.type: Easing.InOutQuad }
                        }
                    }

                    // 左侧指示条
                    Rectangle {
                        anchors.left: parent.left; anchors.leftMargin: 4
                        anchors.verticalCenter: parent.verticalCenter
                        width: 3; height: parent.height - 16; radius: 1.5
                        color: Theme.accent
                        opacity: selected ? 0.9 : 0.0
                        Behavior on opacity { NumberAnimation { duration: 150 } }
                    }

                    // 内容行
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14; anchors.rightMargin: 12; spacing: 8

                        Text {
                            Layout.fillWidth: true; Layout.preferredWidth: 0.45
                            text: m.word
                            color: Theme.textDark; font.pixelSize: 13
                            font.weight: selected ? Font.DemiBold : Font.Medium
                            elide: Text.ElideRight
                        }
                        Text {
                            Layout.fillWidth: showTranslation; Layout.preferredWidth: showTranslation ? 0.3 : 0
                            visible: showTranslation
                            text: m.translation
                            color: Theme.textMid; font.pixelSize: 12; elide: Text.ElideRight
                        }
                        Text {
                            Layout.fillWidth: showPhonetic; Layout.preferredWidth: showPhonetic ? 0.25 : 0
                            visible: showPhonetic
                            text: m.phonetic
                            color: Theme.textLight; font.pixelSize: 11; elide: Text.ElideRight
                        }
                    }

                    // 鼠标交互：左键选择/拖拽，右键取消选中
                    MouseArea {
                        id: itemMouse
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        propagateComposedEvents: false

                        property bool isDragging: false
                        property int startIdx: -1
                        property bool addMode: true
                        property int modifiersOnPress: 0
                        property int pressedButton: Qt.NoButton

                        // 自动滚动定时器
                        Timer {
                            id: autoScroll
                            interval: 16
                            repeat: true
                            running: itemMouse.isDragging
                            onTriggered: {
                                var mouseY = itemMouse.mouseY
                                var viewHeight = listView.height
                                var edge = 25
                                var speedPerFrame = 14

                                if (mouseY < edge && listView.contentY > 0) {
                                    listView.contentY = Math.max(0, listView.contentY - speedPerFrame)
                                } else if (mouseY > viewHeight - edge && listView.contentY < listView.contentHeight - listView.height) {
                                    listView.contentY = Math.min(listView.contentHeight - listView.height, listView.contentY + speedPerFrame)
                                }

                                var mapped = listView.contentItem.mapFromItem(itemMouse, itemMouse.mouseX, itemMouse.mouseY)
                                var idx = listView.indexAt(mapped.x, mapped.y)
                                if (idx === -1) {
                                    if (mapped.y < 0) idx = 0
                                    else if (mapped.y > listView.contentHeight) idx = wordModel.count - 1
                                    else return
                                }

                                if (idx !== -1) {
                                    if (itemMouse.addMode) {
                                        selection.addRange(itemMouse.startIdx, idx)
                                    } else {
                                        selection.removeRange(itemMouse.startIdx, idx)
                                    }
                                }
                            }
                        }

                        onPressed: function(mouse) {
                            root.forceActiveFocus() // 让组件获得焦点以响应快捷键
                            pressedButton = mouse.button
                            if (mouse.button === Qt.LeftButton) {
                                isDragging = false
                                startIdx = index
                                modifiersOnPress = mouse.modifiers
                                pressedX = mouse.x
                                pressedY = mouse.y
                            } else {
                                isDragging = false
                            }
                        }

                        onPositionChanged: function(mouse) {
                            if (pressedButton !== Qt.LeftButton || !pressed) return

                            if (!isDragging) {
                                var dist = Math.abs(mouse.x - pressedX) + Math.abs(mouse.y - pressedY)
                                if (dist > 2) {
                                    isDragging = true
                                    dragState.active = true
                                    root.clearOtherList(activeListType)
                                    if (modifiersOnPress & Qt.ControlModifier) {
                                        addMode = !selection.contains(startIdx)
                                    } else {
                                        selection.selectSingle(startIdx)
                                        addMode = true
                                    }
                                }
                            }
                            if (isDragging) {
                                var mapped = listView.contentItem.mapFromItem(itemMouse, mouse.x, mouse.y)
                                var idx = listView.indexAt(mapped.x, mapped.y)
                                if (idx !== -1) {
                                    if (addMode) {
                                        selection.addRange(startIdx, idx)
                                    } else {
                                        selection.removeRange(startIdx, idx)
                                    }
                                }
                            }
                        }

                        onReleased: function(mouse) {
                            if (pressedButton === Qt.RightButton) {
                                if (selection.contains(index)) {
                                    selection.remove(index)
                                }
                                return
                            }

                            if (isDragging) {
                                isDragging = false
                                dragState.active = false
                            } else {
                                if (modifiersOnPress & Qt.ControlModifier) {
                                    selection.toggle(index)
                                    root.clearOtherList(activeListType)
                                } else if (modifiersOnPress & Qt.ShiftModifier) {
                                    var anchor = selection.anchorIndex
                                    if (anchor < 0) {
                                        selection.selectSingle(index)
                                    } else {
                                        selection.selectRange(anchor, index)
                                    }
                                    root.clearOtherList(activeListType)
                                } else {
                                    selection.selectSingle(index)
                                    root.clearOtherList(activeListType)
                                }
                            }
                            pressedButton = Qt.NoButton
                        }

                        onCanceled: {
                            if (isDragging) {
                                isDragging = false
                                dragState.active = false
                            }
                            pressedButton = Qt.NoButton
                        }

                        property real pressedX: 0
                        property real pressedY: 0
                    }
                }

                // 滚动条
                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded; hoverEnabled: true; width: 6
                    anchors.right: parent.right; anchors.rightMargin: 2
                    contentItem: Rectangle {
                        implicitWidth: (parent.hovered || parent.pressed) ? 8 : 4
                        radius: implicitWidth / 2
                        color: {
                            if (parent.pressed) return Theme.accent
                            if (parent.hovered) return Qt.lighter(Theme.accent, 1.3)
                            return Theme.dark ? Qt.rgba(1, 1, 1, 0.25) : Qt.rgba(0, 0, 0, 0.15)
                        }
                        opacity: (parent.hovered || parent.pressed || listView.moving || listView.flicking) ? 1.0 : 0.0
                        Behavior on implicitWidth { NumberAnimation { duration: 200 } }
                        Behavior on opacity { NumberAnimation { duration: 300 } }
                    }
                    background: null
                }
            }

            // 透明遮罩：完全禁止在空白/缝隙区域拖拽滚动列表
            MouseArea {
                id: blockArea
                anchors.fill: listView
                acceptedButtons: Qt.LeftButton
                propagateComposedEvents: true

                // 标记是否在空白区域按下（即不应该传递给 delegate 的事件）
                property bool blocked: false

                onPressed: function(mouse) {
                    var mapped = listView.contentItem.mapFromItem(this, mouse.x, mouse.y)
                    var idx = listView.indexAt(mapped.x, mapped.y)
                    if (idx === -1) {
                        // 点击在空白区域，接管事件，阻止列表滚动
                        blocked = true
                        mouse.accepted = true
                    } else {
                        blocked = false
                        mouse.accepted = false
                    }
                }

                onPositionChanged: function(mouse) {
                    if (blocked) {
                        // 如果在空白区域按下，这里的移动事件必须被接受，防止穿透到 ListView
                        mouse.accepted = true
                    }
                }

                onReleased: {
                    blocked = false
                }

                onCanceled: {
                    blocked = false
                }
            }
        }
    }
}