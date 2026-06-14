import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "transparent"

    // ========== 公共属性 ==========
    property ListModel knownWordsModel: ListModel {}   // 原始数据
    property ListModel filteredModel: ListModel {}     // 筛选后数据
    property var rootWindow: null                      // 主窗口引用（用于弹窗）
    property var selectedIndices: []                   // 当前选中索引（基于 filteredModel）

    signal addWord(string word, string translation, string phonetic)
    signal deleteWord(string word)

    // ========== 辅助 SVG 生成（颜色从 Theme 动态获取） ==========
    function svgIcon(name, color, size = 18) {
        let svg = ""
        const icons = {
            word: `<svg xmlns='http://www.w3.org/2000/svg' width='${size}' height='${size}' viewBox='0 0 24 24' fill='none' stroke='${color}' stroke-width='2'><path d='M2 3h7a3 3 0 0 1 3 3v14a2 2 0 0 0-2-2H2z'/><path d='M22 3h-7a3 3 0 0 0-3 3v14a2 2 0 0 1 2-2h8z'/></svg>`,
            trans: `<svg xmlns='http://www.w3.org/2000/svg' width='${size}' height='${size}' viewBox='0 0 24 24' fill='none' stroke='${color}' stroke-width='2'><path d='M5 8h14M8 3v5M16 3v5'/><line x1='10' y1='13' x2='10' y2='16'/><line x1='14' y1='13' x2='14' y2='16'/><polyline points='18 16 21 19 18 22'/><polyline points='6 16 3 19 6 22'/></svg>`,
            phonetic: `<svg xmlns='http://www.w3.org/2000/svg' width='${size}' height='${size}' viewBox='0 0 24 24' fill='none' stroke='${color}' stroke-width='2'><polygon points='11 5 6 9 2 9 2 15 6 15 11 19 11 5'/><path d='M15.54 8.46a5 5 0 0 1 0 7.07M18.1 5.9a9 9 0 0 1 0 12.2'/></svg>`,
            add: `<svg xmlns='http://www.w3.org/2000/svg' width='${size}' height='${size}' viewBox='0 0 24 24' fill='none' stroke='${color}' stroke-width='2'><line x1='12' y1='5' x2='12' y2='19'/><line x1='5' y1='12' x2='19' y2='12'/></svg>`,
            delete: `<svg xmlns='http://www.w3.org/2000/svg' width='${size}' height='${size}' viewBox='0 0 24 24' fill='none' stroke='${color}' stroke-width='2'><polyline points='3 6 5 6 21 6'/><path d='M8 6V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2'/><path d='M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6'/></svg>`,
            search: `<svg xmlns='http://www.w3.org/2000/svg' width='${size}' height='${size}' viewBox='0 0 24 24' fill='none' stroke='${color}' stroke-width='2'><circle cx='11' cy='11' r='8'/><line x1='21' y1='21' x2='16.65' y2='16.65'/></svg>`,
            clear: `<svg xmlns='http://www.w3.org/2000/svg' width='${size}' height='${size}' viewBox='0 0 24 24' fill='none' stroke='${color}' stroke-width='2'><line x1='18' y1='6' x2='6' y2='18'/><line x1='6' y1='6' x2='18' y2='18'/></svg>`,
            apply: `<svg xmlns='http://www.w3.org/2000/svg' width='${size}' height='${size}' viewBox='0 0 24 24' fill='none' stroke='${color}' stroke-width='2'><polyline points='20 6 9 17 4 12'/></svg>`,
            import: `<svg xmlns='http://www.w3.org/2000/svg' width='${size}' height='${size}' viewBox='0 0 24 24' fill='none' stroke='${color}' stroke-width='2'><path d='M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4'/><polyline points='7 10 12 15 17 10'/><line x1='12' y1='15' x2='12' y2='3'/></svg>`,
            export: `<svg xmlns='http://www.w3.org/2000/svg' width='${size}' height='${size}' viewBox='0 0 24 24' fill='none' stroke='${color}' stroke-width='2'><path d='M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4'/><polyline points='7 10 12 15 17 10'/><line x1='12' y1='15' x2='12' y2='3'/></svg>`,
            csv: `<svg xmlns='http://www.w3.org/2000/svg' width='${size}' height='${size}' viewBox='0 0 24 24' fill='none' stroke='${color}' stroke-width='2'><path d='M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z'/><polyline points='14 2 14 8 20 8'/><line x1='8' y1='13' x2='16' y2='13'/><line x1='8' y1='17' x2='16' y2='17'/><polyline points='10 9 9 9 8 9'/></svg>`,
            json: `<svg xmlns='http://www.w3.org/2000/svg' width='${size}' height='${size}' viewBox='0 0 24 24' fill='none' stroke='${color}' stroke-width='2'><path d='M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z'/><polyline points='14 2 14 8 20 8'/><path d='M10 12h1v5'/><path d='M14 12h1v5'/><path d='M7 12h1v5'/><path d='M19 12h1v5'/></svg>`,
            stats: `<svg xmlns='http://www.w3.org/2000/svg' width='${size}' height='${size}' viewBox='0 0 24 24' fill='none' stroke='${color}' stroke-width='2'><line x1='18' y1='20' x2='18' y2='10'/><line x1='12' y1='20' x2='12' y2='4'/><line x1='6' y1='20' x2='6' y2='14'/></svg>`,
            refresh: `<svg xmlns='http://www.w3.org/2000/svg' width='${size}' height='${size}' viewBox='0 0 24 24' fill='none' stroke='${color}' stroke-width='2'><path d='M21 2v6h-6M3 12a9 9 0 0 1 15-6.7L21 8M3 22v-6h6M21 12a9 9 0 0 1-15 6.7L3 16'/></svg>`,
            logo: `<svg xmlns='http://www.w3.org/2000/svg' width='${size}' height='${size}' viewBox='0 0 24 24' fill='none' stroke='${color}' stroke-width='2'><circle cx='12' cy='12' r='10'/><polyline points='8 12 11 15 16 9'/></svg>`
        }
        svg = icons[name] || icons["logo"]
        return `data:image/svg+xml;utf8,${encodeURIComponent(svg)}`
    }

    // ========== 筛选逻辑 ==========
    function rebuildAdvancedFilter() {
        filteredModel.clear()
        let keyword = searchField.text
        let caseOn = caseSensitive.checked
        let exact = exactMatch.checked
        let field = fieldSelector.currentIndex

        function match(src) {
            let s = caseOn ? src : src.toLowerCase()
            let k = caseOn ? keyword : keyword.toLowerCase()
            return exact ? s === k : s.indexOf(k) !== -1
        }

        for (let i = 0; i < knownWordsModel.count; ++i) {
            let item = knownWordsModel.get(i)
            let ok = false
            if (field === 0) ok = match(item.word) || match(item.translation) || match(item.phonetic)
            else if (field === 1) ok = match(item.word)
            else if (field === 2) ok = match(item.translation)
            else if (field === 3) ok = match(item.phonetic)
            if (ok) filteredModel.append(item)
        }
        updateStats()
    }

    Connections {
        target: knownWordsModel
        function onRowsInserted() { rebuildAdvancedFilter() }
        function onRowsRemoved() { rebuildAdvancedFilter() }
    }
    Component.onCompleted: rebuildAdvancedFilter()

    // ========== 导出函数 ==========
    function exportCSV() {
        let csv = "word,translation,phonetic\n"
        for (let i = 0; i < knownWordsModel.count; ++i) {
            let item = knownWordsModel.get(i)
            csv += `${item.word},${item.translation},${item.phonetic}\n`
        }
        if (rootWindow) {
            let dialog = Qt.createComponent("MessageDialog.qml")
            if (dialog.status === Component.Ready) {
                let dlg = dialog.createObject(rootWindow)
                dlg.dialogTitle = "导出 CSV"
                dlg.dialogText = csv
                dlg.standardButtons = "ok"
                dlg.openCentered(rootWindow)
            }
        } else console.log(csv)
    }

    function exportJSON() {
        let arr = []
        for (let i = 0; i < knownWordsModel.count; ++i) arr.push(knownWordsModel.get(i))
        let json = JSON.stringify(arr, null, 2)
        if (rootWindow) {
            let dialog = Qt.createComponent("MessageDialog.qml")
            if (dialog.status === Component.Ready) {
                let dlg = dialog.createObject(rootWindow)
                dlg.dialogTitle = "导出 JSON"
                dlg.dialogText = json
                dlg.standardButtons = "ok"
                dlg.openCentered(rootWindow)
            }
        } else console.log(json)
    }

    function updateStats() {
        statText.text = `总词数: ${knownWordsModel.count}  ·  当前显示: ${filteredModel.count}`
    }

    // ========== 单词列表内部状态（多选、拖拽） ==========
    QtObject {
        id: listInternal
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

    function isSelected(idx) { return selectedIndices.indexOf(idx) !== -1 }
    function applySelection(newIndices, newAnchor) {
        selectedIndices = newIndices.slice()
        anchorIndex = newAnchor
    }
    function singleSelect(idx) { applySelection([idx], idx) }
    function toggleSelect(idx) {
        let arr = selectedIndices.slice()
        let pos = arr.indexOf(idx)
        if (pos !== -1) arr.splice(pos, 1)
        else arr.push(idx)
        arr.sort((a,b)=>a-b)
        applySelection(arr, idx)
    }
    function rangeSelect(idx) {
        let anchor = anchorIndex !== -1 ? anchorIndex : idx
        let start = Math.min(anchor, idx), end = Math.max(anchor, idx)
        let arr = []
        for (let i = start; i <= end; i++) arr.push(i)
        applySelection(arr, anchor)
    }
    function selectAll() {
        let arr = []
        for (let i = 0; i < filteredModel.count; i++) arr.push(i)
        applySelection(arr, filteredModel.count > 0 ? filteredModel.count-1 : -1)
    }

    property int anchorIndex: -1

    function startDrag(startIdx, addMode) {
        listInternal.isDragging = true
        listInternal.dragStartIdx = startIdx
        listInternal.dragLastIdx = startIdx
        listInternal.dragAddMode = addMode
        listInternal.dragOriginal = selectedIndices.slice()
        applyDragTo(startIdx)
    }
    function applyDragTo(currentIdx) {
        let start = Math.min(listInternal.dragStartIdx, currentIdx)
        let end = Math.max(listInternal.dragStartIdx, currentIdx)
        let newArr = []
        if (listInternal.dragAddMode) {
            let base = listInternal.dragOriginal.slice()
            for (let i=start; i<=end; i++)
                if (base.indexOf(i) === -1) base.push(i)
            newArr = base
        } else {
            for (let j=0; j<listInternal.dragOriginal.length; j++) {
                let idx = listInternal.dragOriginal[j]
                if (idx < start || idx > end) newArr.push(idx)
            }
        }
        newArr.sort((a,b)=>a-b)
        applySelection(newArr, currentIdx)
        listInternal.dragLastIdx = currentIdx
    }
    function endDrag() {
        listInternal.isDragging = false
        listInternal.dragStartIdx = -1
        listInternal.dragLastIdx = -1
        listInternal.dragOriginal = []
    }

    // ========== 主布局 ==========
    RowLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // 左侧：单词列表卡片（占比加大）
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 18
            color: "transparent"

            // 阴影
            Rectangle {
                anchors.fill: parent; anchors.margins: -2; radius: 18
                color: Qt.rgba(0,0,0,Theme.dark ? 0.3 : 0.06); z: -1
            }
            Rectangle {
                anchors.fill: parent; anchors.margins: -4; radius: 18
                color: Qt.rgba(0,0,0,Theme.dark ? 0.15 : 0.03); z: -2
            }

            // 卡片背景
            Rectangle {
                anchors.fill: parent; radius: 18
                color: Theme.surface
                border.color: Qt.rgba(Theme.borderInner.r, Theme.borderInner.g, Theme.borderInner.b, 0.5)
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    // 标题行
                    RowLayout {
                        Image {
                            source: svgIcon("logo", Theme.accent, 18)
                            width: 18; height: 18
                        }
                        Text {
                            text: "熟词库"
                            font.pixelSize: 16; font.bold: true
                            color: Theme.textDark
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: `${filteredModel.count} 项`
                            font.pixelSize: 11
                            color: Theme.textLight
                        }
                    }

                    // 列表视图
                    ListView {
                        id: listView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: filteredModel
                        clip: true
                        interactive: !listInternal.isDragging

                        // 表头
                        header: RowLayout {
                            width: listView.width
                            height: 32
                            spacing: 8
                            Rectangle {
                                Layout.fillWidth: true; Layout.preferredWidth: 0.45
                                height: 28; radius: 6
                                color: Theme.dark ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.03)
                                Text {
                                    anchors.centerIn: parent
                                    text: "单词"; font.pixelSize: 11; font.bold: true
                                    color: Theme.textMid
                                }
                            }
                            Rectangle {
                                Layout.fillWidth: true; Layout.preferredWidth: 0.3
                                height: 28; radius: 6
                                color: Theme.dark ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.03)
                                Text {
                                    anchors.centerIn: parent
                                    text: "释义"; font.pixelSize: 11; font.bold: true
                                    color: Theme.textMid
                                }
                            }
                            Rectangle {
                                Layout.fillWidth: true; Layout.preferredWidth: 0.25
                                height: 28; radius: 6
                                color: Theme.dark ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.03)
                                Text {
                                    anchors.centerIn: parent
                                    text: "音标"; font.pixelSize: 11; font.bold: true
                                    color: Theme.textMid
                                }
                            }
                        }

                        delegate: Rectangle {
                            id: delegateItem
                            width: listView.width
                            height: 48
                            color: "transparent"
                            property bool selected: isSelected(index)

                            // 选中背景
                            Rectangle {
                                anchors.fill: parent; radius: 8
                                color: Theme.accent
                                opacity: selected ? (Theme.dark ? 0.12 : 0.18) : 0
                                Behavior on opacity { NumberAnimation { duration: 150 } }
                            }
                            // 左侧指示条
                            Rectangle {
                                width: 3; height: parent.height-16; radius: 1.5
                                anchors.left: parent.left; anchors.leftMargin: 6
                                anchors.verticalCenter: parent.verticalCenter
                                color: Theme.accent
                                opacity: selected ? 0.9 : 0
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 16
                                anchors.rightMargin: 12
                                spacing: 8

                                Text {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 0.45
                                    text: model.word
                                    color: Theme.textDark
                                    font.pixelSize: 13
                                    font.weight: selected ? Font.DemiBold : Font.Medium
                                    elide: Text.ElideRight
                                }
                                Text {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 0.3
                                    text: model.translation
                                    color: Theme.textMid
                                    font.pixelSize: 12
                                    elide: Text.ElideRight
                                }
                                Text {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 0.25
                                    text: model.phonetic
                                    color: Theme.textLight
                                    font.pixelSize: 11
                                    elide: Text.ElideRight
                                }
                            }

                            Rectangle {
                                anchors.bottom: parent.bottom
                                width: parent.width - 24; height: 1
                                color: Theme.borderInner
                                opacity: 0.3
                                visible: index !== filteredModel.count-1
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton
                                property point pressPoint

                                onPressed: (mouse) => {
                                    root.forceActiveFocus()
                                    let mapped = listView.contentItem.mapFromItem(delegateItem, mouse.x, mouse.y)
                                    listInternal.pressedIdx = index
                                    listInternal.pressStartPos = Qt.point(mapped.x, mapped.y)
                                    listInternal.isDragging = false
                                    listInternal.pressedModifiers = mouse.modifiers
                                    listInternal.pressedSelected = selected
                                    if (mouse.modifiers & Qt.ShiftModifier) rangeSelect(index)
                                }
                                onPositionChanged: (mouse) => {
                                    if (listInternal.pressedIdx === -1) return
                                    let mapped = listView.contentItem.mapFromItem(delegateItem, mouse.x, mouse.y)
                                    let dist = Math.abs(mapped.x - listInternal.pressStartPos.x) + Math.abs(mapped.y - listInternal.pressStartPos.y)
                                    if (dist > 2 && !listInternal.isDragging) {
                                        let addMode = true
                                        if (listInternal.pressedModifiers & Qt.ControlModifier) {
                                            addMode = !listInternal.pressedSelected
                                        } else if (!(listInternal.pressedModifiers & Qt.ShiftModifier)) {
                                            singleSelect(listInternal.pressedIdx)
                                        }
                                        startDrag(listInternal.pressedIdx, addMode)
                                    }
                                    if (listInternal.isDragging) {
                                        let curIdx = listView.indexAt(mapped.x, mapped.y)
                                        if (curIdx !== -1 && curIdx !== listInternal.dragLastIdx)
                                            applyDragTo(curIdx)
                                    }
                                }
                                onReleased: (mouse) => {
                                    if (listInternal.isDragging) { endDrag(); return }
                                    let mapped = listView.contentItem.mapFromItem(delegateItem, mouse.x, mouse.y)
                                    let idx = listView.indexAt(mapped.x, mapped.y)
                                    if (idx === -1) return
                                    if (mouse.modifiers & Qt.ControlModifier) toggleSelect(idx)
                                    else if (mouse.modifiers & Qt.ShiftModifier) rangeSelect(idx)
                                    else singleSelect(idx)
                                }
                            }
                        }

                        // 滚动条
                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                            width: 6
                            contentItem: Rectangle {
                                radius: 3
                                color: Theme.dark ? Qt.rgba(1,1,1,0.25) : Qt.rgba(0,0,0,0.15)
                                opacity: 0.6
                            }
                        }
                    }
                }
            }
        }

        // 右侧：操作面板（宽度固定 260，使左侧更大）
        Rectangle {
            Layout.preferredWidth: 260
            Layout.fillHeight: true
            radius: 18
            color: "transparent"

            Rectangle {
                anchors.fill: parent; anchors.margins: -2; radius: 18
                color: Qt.rgba(0,0,0,Theme.dark ? 0.3 : 0.06); z: -1
            }
            Rectangle {
                anchors.fill: parent; anchors.margins: -4; radius: 18
                color: Qt.rgba(0,0,0,Theme.dark ? 0.15 : 0.03); z: -2
            }

            Rectangle {
                anchors.fill: parent; radius: 18
                color: Theme.surface
                border.color: Qt.rgba(Theme.borderInner.r, Theme.borderInner.g, Theme.borderInner.b, 0.5)
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 14

                    Text {
                        text: "操作面板"
                        font.pixelSize: 15; font.bold: true
                        color: Theme.textDark
                        Layout.alignment: Qt.AlignHCenter
                    }

                    // TabBar (纯 SVG 图标)
                    TabBar {
                        id: tabBar
                        Layout.fillWidth: true
                        background: Rectangle {
                            color: "transparent"
                            radius: 12
                            border.color: Qt.rgba(Theme.borderInner.r, Theme.borderInner.g, Theme.borderInner.b, 0.3)
                        }

                        TabButton {
                            contentItem: RowLayout {
                                Image { source: svgIcon("word", tabBar.currentIndex===0 ? Theme.accent : Theme.textMid, 14); width: 14; height: 14 }
                                Text { text: "基础"; color: tabBar.currentIndex===0 ? Theme.textDark : Theme.textMid; font.pixelSize: 11 }
                            }
                            background: Rectangle { color: parent.checked ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.1) : "transparent"; radius: 10 }
                        }
                        TabButton {
                            contentItem: RowLayout {
                                Image { source: svgIcon("search", tabBar.currentIndex===1 ? Theme.accent : Theme.textMid, 14); width: 14; height: 14 }
                                Text { text: "查找"; color: tabBar.currentIndex===1 ? Theme.textDark : Theme.textMid; font.pixelSize: 11 }
                            }
                            background: Rectangle { color: parent.checked ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.1) : "transparent"; radius: 10 }
                        }
                        TabButton {
                            contentItem: RowLayout {
                                Image { source: svgIcon("import", tabBar.currentIndex===2 ? Theme.accent : Theme.textMid, 14); width: 14; height: 14 }
                                Text { text: "导入/出"; color: tabBar.currentIndex===2 ? Theme.textDark : Theme.textMid; font.pixelSize: 11 }
                            }
                            background: Rectangle { color: parent.checked ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.1) : "transparent"; radius: 10 }
                        }
                        TabButton {
                            contentItem: RowLayout {
                                Image { source: svgIcon("stats", tabBar.currentIndex===3 ? Theme.accent : Theme.textMid, 14); width: 14; height: 14 }
                                Text { text: "统计"; color: tabBar.currentIndex===3 ? Theme.textDark : Theme.textMid; font.pixelSize: 11 }
                            }
                            background: Rectangle { color: parent.checked ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.1) : "transparent"; radius: 10 }
                        }
                    }

                    StackLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        currentIndex: tabBar.currentIndex

                        // ----- 基础操作 -----
                        ColumnLayout {
                            spacing: 12
                            RowLayout {
                                Image { source: svgIcon("word", Theme.textMid, 16); width: 16; height: 16 }
                                TextField {
                                    id: newWordField
                                    Layout.fillWidth: true
                                    placeholderText: "单词"
                                    color: Theme.textDark
                                    font.pixelSize: 12
                                    background: Rectangle { radius: 10; color: Theme.surface; border.color: Theme.borderInner }
                                }
                            }
                            RowLayout {
                                Image { source: svgIcon("trans", Theme.textMid, 16); width: 16; height: 16 }
                                TextField {
                                    id: newTransField
                                    Layout.fillWidth: true
                                    placeholderText: "释义"
                                    color: Theme.textDark
                                    font.pixelSize: 12
                                    background: Rectangle { radius: 10; color: Theme.surface; border.color: Theme.borderInner }
                                }
                            }
                            RowLayout {
                                Image { source: svgIcon("phonetic", Theme.textMid, 16); width: 16; height: 16 }
                                TextField {
                                    id: newPhoneticField
                                    Layout.fillWidth: true
                                    placeholderText: "音标"
                                    color: Theme.textDark
                                    font.pixelSize: 12
                                    background: Rectangle { radius: 10; color: Theme.surface; border.color: Theme.borderInner }
                                }
                            }

                            Button {
                                text: "添加单词"
                                Layout.fillWidth: true; Layout.preferredHeight: 34
                                icon.source: svgIcon("add", "#FFFFFF", 14)
                                icon.width: 14; icon.height: 14
                                background: Rectangle {
                                    radius: 12
                                    color: parent.hovered ? Qt.lighter(Theme.btnPrimaryBg, 1.1) : Theme.btnPrimaryBg
                                    border.color: Theme.btnPrimaryBorder
                                }
                                contentItem: Text { text: parent.text; color: "#FFFFFF"; font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter }
                                onClicked: {
                                    let word = newWordField.text.trim().toLowerCase()
                                    if (word === "") { if (rootWindow) rootWindow.statusTextValue = "请输入单词"; return }
                                    for (let i=0; i<knownWordsModel.count; ++i)
                                        if (knownWordsModel.get(i).word.toLowerCase() === word) { if (rootWindow) rootWindow.statusTextValue = "单词已存在"; return }
                                    root.addWord(word, newTransField.text.trim() || "待补充", newPhoneticField.text.trim())
                                    newWordField.clear(); newTransField.clear(); newPhoneticField.clear()
                                    if (rootWindow) rootWindow.statusTextValue = `已添加: ${word}`
                                }
                            }

                            Button {
                                text: "删除选中"
                                Layout.fillWidth: true; Layout.preferredHeight: 34
                                icon.source: svgIcon("delete", "#FFFFFF", 14)
                                icon.width: 14; icon.height: 14
                                background: Rectangle {
                                    radius: 12
                                    color: parent.hovered ? Qt.lighter(Theme.btnDangerBg, 1.1) : Theme.btnDangerBg
                                    border.color: Theme.btnDangerBorder
                                }
                                contentItem: Text { text: parent.text; color: "#FFFFFF"; font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter }
                                onClicked: {
                                    if (selectedIndices.length === 0) {
                                        if (rootWindow) rootWindow.statusTextValue = "未选中任何单词"
                                        return
                                    }
                                    function doDelete() {
                                        let sorted = selectedIndices.slice().sort((a,b)=>b-a)
                                        for (let idx of sorted) root.deleteWord(filteredModel.get(idx).word)
                                    }
                                    if (rootWindow) {
                                        let dialog = Qt.createComponent("MessageDialog.qml")
                                        if (dialog.status === Component.Ready) {
                                            let dlg = dialog.createObject(rootWindow)
                                            dlg.dialogTitle = "批量删除"
                                            dlg.dialogText = `确认删除选中的 ${selectedIndices.length} 个单词？`
                                            dlg.standardButtons = "okcancel"
                                            dlg.accepted.connect(function() { doDelete(); dlg.destroy() })
                                            dlg.openCentered(rootWindow)
                                        } else doDelete()
                                    } else doDelete()
                                }
                            }
                        }

                        // ----- 高级查找 -----
                        ColumnLayout {
                            spacing: 12
                            RowLayout {
                                Image { source: svgIcon("search", Theme.textMid, 16); width: 16; height: 16 }
                                TextField {
                                    id: searchField
                                    Layout.fillWidth: true
                                    placeholderText: "输入关键词..."
                                    color: Theme.textDark
                                    font.pixelSize: 12
                                    background: Rectangle { radius: 10; color: Theme.surface; border.color: Theme.borderInner }
                                    onTextChanged: rebuildAdvancedFilter()
                                }
                            }
                            ComboBox {
                                id: fieldSelector
                                Layout.fillWidth: true
                                model: ["全部字段", "单词", "释义", "音标"]
                                background: Rectangle { radius: 10; color: Theme.surface; border.color: Theme.borderInner }
                                contentItem: Text { text: parent.displayText; color: Theme.textDark; font.pixelSize: 12; leftPadding: 10 }
                            }
                            CheckBox {
                                id: caseSensitive; text: "区分大小写"
                                contentItem: Text { text: parent.text; color: Theme.textMid; font.pixelSize: 11 }
                                indicator: Rectangle {
                                    implicitWidth: 16; implicitHeight: 16; radius: 4
                                    color: parent.checked ? Theme.btnPrimaryBg : Theme.surface
                                    border.color: Theme.borderInner
                                }
                            }
                            CheckBox {
                                id: exactMatch; text: "精确匹配"
                                contentItem: Text { text: parent.text; color: Theme.textMid; font.pixelSize: 11 }
                                indicator: Rectangle {
                                    implicitWidth: 16; implicitHeight: 16; radius: 4
                                    color: parent.checked ? Theme.btnPrimaryBg : Theme.surface
                                    border.color: Theme.borderInner
                                }
                            }
                            RowLayout {
                                spacing: 8
                                Button {
                                    text: "应用"; Layout.fillWidth: true
                                    icon.source: svgIcon("apply", "#FFFFFF", 12); icon.width: 12; icon.height: 12
                                    background: Rectangle { radius: 10; color: parent.hovered ? Qt.lighter(Theme.btnPrimaryBg,1.1) : Theme.btnPrimaryBg; border.color: Theme.btnPrimaryBorder }
                                    contentItem: Text { text: parent.text; color: "#FFFFFF"; font.pixelSize: 11 }
                                    onClicked: rebuildAdvancedFilter()
                                }
                                Button {
                                    text: "清空"; Layout.fillWidth: true
                                    icon.source: svgIcon("clear", Theme.textDark, 12); icon.width: 12; icon.height: 12
                                    background: Rectangle { radius: 10; color: parent.hovered ? Qt.lighter(Theme.btnNormalBg,1.1) : Theme.btnNormalBg; border.color: Theme.btnNormalBorder }
                                    contentItem: Text { text: parent.text; color: Theme.textDark; font.pixelSize: 11 }
                                    onClicked: searchField.clear()
                                }
                            }
                        }

                        // ----- 导入导出 -----
                        ColumnLayout {
                            spacing: 12
                            Button {
                                text: "导入 CSV"; Layout.fillWidth: true; Layout.preferredHeight: 36
                                icon.source: svgIcon("import", Theme.textDark, 14); icon.width: 14; icon.height: 14
                                background: Rectangle { radius: 12; color: parent.hovered ? Qt.lighter(Theme.btnNormalBg,1.1) : Theme.btnNormalBg; border.color: Theme.btnNormalBorder }
                                contentItem: Text { text: parent.text; color: Theme.textDark; font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter }
                                onClicked: { if (rootWindow) rootWindow.showDemoDialog("导入", "导入 CSV 功能演示") }
                            }
                            Button {
                                text: "导出 CSV"; Layout.fillWidth: true; Layout.preferredHeight: 36
                                icon.source: svgIcon("csv", Theme.textDark, 14); icon.width: 14; icon.height: 14
                                background: Rectangle { radius: 12; color: parent.hovered ? Qt.lighter(Theme.btnNormalBg,1.1) : Theme.btnNormalBg; border.color: Theme.btnNormalBorder }
                                contentItem: Text { text: parent.text; color: Theme.textDark; font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter }
                                onClicked: exportCSV()
                            }
                            Button {
                                text: "导出 JSON"; Layout.fillWidth: true; Layout.preferredHeight: 36
                                icon.source: svgIcon("json", Theme.textDark, 14); icon.width: 14; icon.height: 14
                                background: Rectangle { radius: 12; color: parent.hovered ? Qt.lighter(Theme.btnNormalBg,1.1) : Theme.btnNormalBg; border.color: Theme.btnNormalBorder }
                                contentItem: Text { text: parent.text; color: Theme.textDark; font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter }
                                onClicked: exportJSON()
                            }
                        }

                        // ----- 统计 -----
                        ColumnLayout {
                            spacing: 16
                            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                            Text {
                                id: statText
                                text: `总词数: ${knownWordsModel.count}  ·  当前显示: ${filteredModel.count}`
                                font.pixelSize: 12; color: Theme.textDark
                                horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true; wrapMode: Text.WordWrap
                            }
                            Button {
                                text: "刷新统计"; Layout.fillWidth: true; Layout.preferredHeight: 34
                                icon.source: svgIcon("refresh", Theme.textDark, 14); icon.width: 14; icon.height: 14
                                background: Rectangle { radius: 12; color: parent.hovered ? Qt.lighter(Theme.btnNormalBg,1.1) : Theme.btnNormalBg; border.color: Theme.btnNormalBorder }
                                contentItem: Text { text: parent.text; color: Theme.textDark; font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter }
                                onClicked: updateStats()
                            }
                        }
                    }
                }
            }
        }
    }

    // 全选快捷键
    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_A && event.modifiers & Qt.ControlModifier) {
            selectAll()
            event.accepted = true
        }
    }
}