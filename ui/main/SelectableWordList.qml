import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property string title: "列表"
    property string iconColor: "#000000"
    property string iconSvg: ""
    property ListModel wordModel
    property var selectedIndices: []
    property int anchorIndex: -1
    property string activeListType: ""
    property bool showTranslation: false
    property bool showPhonetic: false

    signal selectionChanged(var indices)
    signal clearOtherLists(string activeType)

    // 全选功能
    function selectAll() {
        if (!wordModel) return
        var arr = []
        for (var i = 0; i < wordModel.count; i++) {
            arr.push(i)
        }
        applySelection(arr, wordModel.count > 0 ? wordModel.count - 1 : -1)
    }

    // 响应 Ctrl+A 全选
    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_A && event.modifiers & Qt.ControlModifier) {
            selectAll()
            event.accepted = true
        }
    }

    QtObject {
        id: internal
        property bool isDragging: false
        property int dragStartIdx: -1
        property bool dragAddMode: true
        property var dragOriginal: []
        property int dragLastIdx: -1
        property point pressStartPos: Qt.point(0,0)
        property int pressedModifiers: 0
        property bool pressedSelected: false
        property int pressedIdx: -1
    }

    function clearSelection() { selectedIndices = []; anchorIndex = -1 }
    function isSelected(index) { return selectedIndices.indexOf(index) !== -1 }
    function applySelection(newIndices, newAnchor) {
        selectedIndices = newIndices.slice()
        anchorIndex = newAnchor
        selectionChanged(selectedIndices)
    }
    function clearOtherList(listType) { clearOtherLists(listType) }
    function singleSelect(index) { applySelection([index], index) }
    function toggleSelect(index) {
        var arr = selectedIndices.slice()
        var pos = arr.indexOf(index)
        if (pos !== -1) arr.splice(pos, 1)
        else arr.push(index)
        arr.sort((a,b)=>a-b)
        applySelection(arr, index)
    }
    function rangeSelect(index) {
        var anchor = anchorIndex !== -1 ? anchorIndex : index
        var start = Math.min(anchor, index), end = Math.max(anchor, index)
        var arr = []
        for (var i = start; i <= end; i++) arr.push(i)
        applySelection(arr, anchor)
    }

    ColumnLayout {
        anchors.fill: parent; spacing: 8

        Row {
            spacing: 6
            Image {
                width: 16; height: 16; anchors.verticalCenter: parent.verticalCenter
                source: iconSvg ? iconSvg.replace("currentColor", "%23" + iconColor.substring(1)) : ""
                sourceSize: Qt.size(16,16); smooth: true
            }
            Text { text: title; font.pixelSize: 14; font.bold: true; color: iconColor }
        }

        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; color: "transparent"

            ListView {
                id: listView; anchors.fill: parent; model: wordModel; clip: true
                interactive: !internal.isDragging

                // 自动滚动定时器（速度更快）
                Timer {
                    id: autoScrollTimer
                    interval: 16
                    repeat: true
                    property int direction: 0
                    property real step: 0
                    onTriggered: {
                        if (direction !== 0 && step > 0 && listView.contentHeight > listView.height) {
                            var newY = listView.contentY + direction * step
                            newY = Math.max(0, Math.min(newY, listView.contentHeight - listView.height))
                            listView.contentY = newY
                        } else {
                            stop()
                        }
                    }
                }

                header: RowLayout {
                    width: listView.width
                    height: 30
                    spacing: 0
                    anchors.leftMargin: 14

                    Rectangle {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.preferredWidth: 0.3
                        radius: 6
                        color: Theme.dark ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.03)
                        visible: true
                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 4
                            anchors.verticalCenter: parent.verticalCenter
                            text: "单词"
                            font.pixelSize: 12
                            font.bold: true
                            color: Theme.textMid
                            horizontalAlignment: Text.AlignLeft
                        }
                    }

                    Rectangle {
                        Layout.fillHeight: true
                        Layout.fillWidth: showTranslation
                        Layout.preferredWidth: showTranslation ? 0.4 : 0
                        radius: 6
                        color: Theme.dark ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.03)
                        visible: showTranslation
                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 4
                            anchors.verticalCenter: parent.verticalCenter
                            text: "释义"
                            font.pixelSize: 12
                            font.bold: true
                            color: Theme.textMid
                            horizontalAlignment: Text.AlignLeft
                        }
                    }

                    Rectangle {
                        Layout.fillHeight: true
                        Layout.fillWidth: showPhonetic
                        Layout.preferredWidth: showPhonetic ? 0.3 : 0
                        radius: 6
                        color: Theme.dark ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.03)
                        visible: showPhonetic
                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 4
                            anchors.verticalCenter: parent.verticalCenter
                            text: "音标"
                            font.pixelSize: 12
                            font.bold: true
                            color: Theme.textMid
                            horizontalAlignment: Text.AlignLeft
                        }
                    }
                }

                delegate: Rectangle {
                    width: listView.width
                    height: showTranslation ? 44 : 40
                    radius: 6
                    color: "transparent"
                    property bool selected: isSelected(index)

                    Rectangle {
                        anchors.fill: parent; radius: 6; color: Theme.accent
                        opacity: selected ? (Theme.dark ? 0.12 : 0.18) : 0.0
                        Behavior on opacity { NumberAnimation { duration:250; easing.type:Easing.OutCubic } }
                    }
                    Rectangle {
                        anchors.left: parent.left; anchors.leftMargin: 4
                        anchors.verticalCenter: parent.verticalCenter
                        width: 3; height: parent.height-16; radius: 1.5; color: Theme.accent
                        opacity: selected ? 0.9 : 0.0
                        Behavior on opacity { NumberAnimation { duration:250; easing.type:Easing.OutCubic } }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        spacing: 4

                        Text {
                            Layout.fillWidth: true
                            Layout.preferredWidth: 0.3
                            text: (model && model.word !== undefined) ? model.word : ""
                            color: Theme.textDark
                            font.pixelSize: 13
                            font.weight: selected ? Font.DemiBold : Font.Medium
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignLeft
                        }

                        Text {
                            visible: showTranslation
                            Layout.fillWidth: showTranslation
                            Layout.preferredWidth: showTranslation ? 0.4 : 0
                            text: (model && model.translation !== undefined) ? model.translation : ""
                            color: Theme.textMid
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignLeft
                        }

                        Text {
                            visible: showPhonetic
                            Layout.fillWidth: showPhonetic
                            Layout.preferredWidth: showPhonetic ? 0.3 : 0
                            text: (model && model.phonetic !== undefined) ? model.phonetic : ""
                            color: Theme.textLight
                            font.pixelSize: 11
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignLeft
                        }
                    }

                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width-16
                        height: 1
                        color: Qt.rgba(Theme.borderInner.r, Theme.borderInner.g, Theme.borderInner.b, 0.3)
                        visible: index !== wordModel.count-1
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                // 滚动条：默认隐藏，交互时显示，1秒淡出
                ScrollBar.vertical: ScrollBar {
                    id: verticalScrollBar
                    policy: ScrollBar.AsNeeded
                    hoverEnabled: true
                    width: 6
                    anchors.right: parent.right
                    anchors.rightMargin: 2

                    contentItem: Rectangle {
                        implicitWidth: (verticalScrollBar.hovered || verticalScrollBar.pressed) ? 8 : 4
                        radius: implicitWidth / 2

                        color: {
                            if (verticalScrollBar.pressed)
                                return Theme.accent
                            if (verticalScrollBar.hovered)
                                return Qt.lighter(Theme.accent, 1.3)
                            return Theme.dark ? Qt.rgba(1, 1, 1, 0.25) : Qt.rgba(0, 0, 0, 0.15)
                        }

                        opacity: (verticalScrollBar.hovered
                                  || verticalScrollBar.pressed
                                  || autoScrollTimer.running
                                  || listView.moving
                                  || listView.flicking) ? 1.0 : 0.0

                        Behavior on implicitWidth { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }
                        Behavior on opacity       { NumberAnimation { duration: 1000; easing.type: Easing.OutCubic } }
                    }

                    background: null
                }

                // 交互层：选择、拖拽、自动滚动、聚焦
                MouseArea {
                    id: listMouseArea
                    anchors {
                        left: parent.left
                        right: parent.right
                        top: parent.top
                        bottom: parent.bottom
                        rightMargin: 8
                    }
                    acceptedButtons: Qt.LeftButton

                    // 拖拽以支持鼠标移出窗口后继续接收移动事件
                    drag.target: dummyDragTarget
                    drag.threshold: 0
                    Item { id: dummyDragTarget; visible: false }

                    function toContentPos(x,y) { return listView.contentItem.mapFromItem(this, x, y) }

                    onPressed: function(mouse) {
                        // 让当前列表获取键盘焦点，以便响应 Ctrl+A
                        root.forceActiveFocus()

                        var mapped = toContentPos(mouse.x, mouse.y)
                        internal.pressedIdx = listView.indexAt(mapped.x, mapped.y)
                        internal.pressStartPos = Qt.point(mapped.x, mapped.y)
                        internal.isDragging = false
                        internal.pressedModifiers = mouse.modifiers
                        if (internal.pressedIdx === -1) {
                            root.clearOtherList(activeListType)
                            return
                        }
                        internal.pressedSelected = isSelected(internal.pressedIdx)
                        if (mouse.modifiers & Qt.ShiftModifier) rangeSelect(internal.pressedIdx)
                    }

                    onPositionChanged: function(mouse) {
                        // 自动滚动（动态速度，最高约1250px/s）
                        if (internal.pressedIdx !== -1) {
                            var mapped = toContentPos(mouse.x, mouse.y)
                            var margin = 50
                            var visibleTop = listView.contentY
                            var visibleBottom = listView.contentY + listView.height

                            var distToTop = mapped.y - visibleTop
                            var distToBottom = visibleBottom - mapped.y

                            var direction = 0
                            var distance = 0
                            if (distToTop < margin) {
                                direction = -1
                                distance = margin - distToTop
                            } else if (distToBottom < margin) {
                                direction = 1
                                distance = margin - distToBottom
                            }

                            if (direction !== 0) {
                                distance = Math.max(0, Math.min(margin, distance))
                                var maxStep = 20   // 提升最高速度
                                var ratio = distance / margin
                                autoScrollTimer.direction = direction
                                autoScrollTimer.step = ratio * maxStep
                                autoScrollTimer.start()
                            } else {
                                autoScrollTimer.stop()
                            }
                        }

                        // 拖拽选择逻辑
                        if (internal.pressedIdx === -1) return
                        var mapped2 = toContentPos(mouse.x, mouse.y)
                        var dist = Math.abs(mapped2.x - internal.pressStartPos.x) + Math.abs(mapped2.y - internal.pressStartPos.y)
                        if (dist > 2) {
                            if (!internal.isDragging) {
                                var addMode = true
                                if (internal.pressedModifiers & Qt.ControlModifier) {
                                    root.clearOtherList(activeListType)
                                    addMode = !internal.pressedSelected
                                } else if (internal.pressedModifiers & Qt.ShiftModifier) {
                                    return
                                } else {
                                    root.clearOtherList(activeListType)
                                    singleSelect(internal.pressedIdx)
                                }
                                startDrag(internal.pressedIdx, addMode)
                            }
                            if (internal.isDragging) {
                                var curIdx = listView.indexAt(mapped2.x, mapped2.y)
                                if (curIdx !== -1 && curIdx !== internal.dragLastIdx) applyDragTo(curIdx)
                            }
                        }
                    }

                    onReleased: function(mouse) {
                        autoScrollTimer.stop()
                        if (internal.isDragging) { endDrag(); return }
                        var mapped = toContentPos(mouse.x, mouse.y)
                        var idx = listView.indexAt(mapped.x, mapped.y)
                        if (idx === -1) return
                        if (mouse.modifiers & Qt.ControlModifier) toggleSelect(idx)
                        else if (mouse.modifiers & Qt.ShiftModifier) rangeSelect(idx)
                        else singleSelect(idx)
                    }

                    onCanceled: {
                        autoScrollTimer.stop()
                        if (internal.isDragging) endDrag()
                    }
                }
            }
        }
    }

    function startDrag(startIdx, addMode) {
        internal.isDragging = true
        internal.dragStartIdx = startIdx
        internal.dragLastIdx = startIdx
        internal.dragAddMode = addMode
        internal.dragOriginal = selectedIndices.slice()
        applyDragTo(startIdx)
    }

    function applyDragTo(currentIdx) {
        var start = Math.min(internal.dragStartIdx, currentIdx)
        var end = Math.max(internal.dragStartIdx, currentIdx)
        var newArr = []
        if (internal.dragAddMode) {
            var base = internal.dragOriginal.slice()
            for (var i = start; i <= end; i++)
                if (base.indexOf(i) === -1) base.push(i)
            newArr = base
        } else {
            for (var j = 0; j < internal.dragOriginal.length; j++) {
                var idx = internal.dragOriginal[j]
                if (idx < start || idx > end) newArr.push(idx)
            }
        }
        newArr.sort((a,b)=>a-b)
        applySelection(newArr, currentIdx)
        internal.dragLastIdx = currentIdx
    }

    function endDrag() {
        internal.isDragging = false
        internal.dragStartIdx = -1
        internal.dragLastIdx = -1
        internal.dragOriginal = []
    }
}