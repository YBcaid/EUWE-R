import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "transparent"

    // 原始数据模型（熟词库）
    property ListModel knownWordsModel: ListModel {}
    // 筛选后的显示模型（绑定给 SelectableWordList）
    property ListModel filteredModel: ListModel {}

    // 外部交互信号
    signal addWord(string word, string translation, string phonetic)
    signal deleteWord(string word)

    // 主窗口引用
    property var rootWindow: null
    // 当前选中的索引列表（用于批量删除）
    property var selectedIndices: []

    // ----------------- 高级筛选逻辑 -----------------
    function rebuildAdvancedFilter() {
        filteredModel.clear()
        var keyword = searchField.text
        var caseOn = caseSensitive.checked
        var exact = exactMatch.checked
        var field = fieldSelector.currentIndex

        function match(src) {
            if (!caseOn) {
                src = src.toLowerCase()
                keyword = keyword.toLowerCase()
            }
            if (exact) return src === keyword
            return src.indexOf(keyword) !== -1
        }

        for (var i = 0; i < knownWordsModel.count; ++i) {
            var item = knownWordsModel.get(i)
            var ok = false
            if (field === 0) // 全部
                ok = match(item.word) || match(item.translation) || match(item.phonetic)
            else if (field === 1)
                ok = match(item.word)
            else if (field === 2)
                ok = match(item.translation)
            else if (field === 3)
                ok = match(item.phonetic)

            if (ok) filteredModel.append(item)
        }
        updateStats()
    }

    // 数据变更时自动刷新
    Connections {
        target: knownWordsModel
        function onRowsInserted() { rebuildAdvancedFilter() }
        function onRowsRemoved() { rebuildAdvancedFilter() }
    }
    Component.onCompleted: rebuildAdvancedFilter()

    // ----------------- 导出函数 -----------------
    function exportCSV() {
        var csv = "word,translation,phonetic\n"
        for (var i = 0; i < knownWordsModel.count; ++i) {
            var item = knownWordsModel.get(i)
            csv += item.word + "," + item.translation + "," + item.phonetic + "\n"
        }
        if (rootWindow) {
            var dialog = Qt.createComponent("MessageDialog.qml")
            if (dialog.status === Component.Ready) {
                var dlg = dialog.createObject(rootWindow)
                dlg.dialogTitle = "导出 CSV"
                dlg.dialogText = csv
                dlg.standardButtons = "ok"
                dlg.openCentered(rootWindow)
            }
        } else console.log(csv)
    }

    function exportJSON() {
        var arr = []
        for (var i = 0; i < knownWordsModel.count; ++i) arr.push(knownWordsModel.get(i))
        var json = JSON.stringify(arr, null, 2)
        if (rootWindow) {
            var dialog = Qt.createComponent("MessageDialog.qml")
            if (dialog.status === Component.Ready) {
                var dlg = dialog.createObject(rootWindow)
                dlg.dialogTitle = "导出 JSON"
                dlg.dialogText = json
                dlg.standardButtons = "ok"
                dlg.openCentered(rootWindow)
            }
        } else console.log(json)
    }

    function updateStats() {
        statText.text = `📊 总词数: ${knownWordsModel.count}  |  当前显示: ${filteredModel.count}`
    }

    // ----------------- 主布局：左侧表格 + 右侧操作面板 -----------------
    RowLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        // 左侧：熟词表格（高级质感卡片）
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 18
            color: "transparent"

            // 模拟阴影层
            Rectangle {
                anchors.fill: parent
                anchors.margins: -2
                radius: 18
                color: Qt.rgba(0,0,0,0.05)
                z: -1
            }
            Rectangle {
                anchors.fill: parent
                anchors.margins: -4
                radius: 18
                color: Qt.rgba(0,0,0,0.03)
                z: -2
            }

            // 主卡片背景
            Rectangle {
                anchors.fill: parent
                radius: 18
                color: Theme.surface
                border.color: Qt.rgba(Theme.borderInner.r, Theme.borderInner.g, Theme.borderInner.b, 0.5)
                border.width: 1
                // 细微渐变增加质感
                gradient: Gradient {
                    GradientStop { position: 0.0; color: Qt.rgba(1,1,1,0.08) }
                    GradientStop { position: 1.0; color: "transparent" }
                }
            }

            SelectableWordList {
                id: wordListComp
                anchors.fill: parent
                anchors.margins: 16
                title: "✨ 熟词库"
                iconColor: "#28a745"
                iconSvg: "data:image/svg+xml;utf8,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'%3E%3Ccircle cx='12' cy='12' r='10'/%3E%3Cpolyline points='8 12 11 15 16 9'/%3E%3C/svg%3E"
                wordModel: filteredModel
                activeListType: "knownWord"
                showTranslation: true
                showPhonetic: true
                onSelectionChanged: function(indices) {
                    root.selectedIndices = indices.slice()
                }
            }
        }

        // 右侧：操作面板（悬浮质感卡片）
        Rectangle {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            radius: 18

            // 模拟阴影层
            Rectangle {
                anchors.fill: parent
                anchors.margins: -2
                radius: 18
                color: Qt.rgba(0,0,0,0.05)
                z: -1
            }
            Rectangle {
                anchors.fill: parent
                anchors.margins: -4
                radius: 18
                color: Qt.rgba(0,0,0,0.03)
                z: -2
            }

            // 主面板
            Rectangle {
                anchors.fill: parent
                radius: 18
                color: Theme.surface
                border.color: Qt.rgba(Theme.borderInner.r, Theme.borderInner.g, Theme.borderInner.b, 0.5)
                border.width: 1
                gradient: Gradient {
                    GradientStop { position: 0.0; color: Qt.rgba(1,1,1,0.06) }
                    GradientStop { position: 1.0; color: "transparent" }
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    // 面板标题
                    Text {
                        text: "⚡ 操作面板"
                        font.pixelSize: 16
                        font.bold: true
                        color: Theme.textDark
                        Layout.alignment: Qt.AlignHCenter
                        Layout.bottomMargin: 4
                    }

                    // TabBar（带底部指示器）
                    TabBar {
                        id: tabBar
                        Layout.fillWidth: true
                        background: Rectangle {
                            color: Theme.surface
                            radius: 12
                            border.color: Qt.rgba(Theme.borderInner.r, Theme.borderInner.g, Theme.borderInner.b, 0.3)
                        }

                        TabButton {
                            text: "📝 基础"
                            font.pixelSize: 11
                            contentItem: Text {
                                text: parent.text
                                color: parent.checked ? Theme.textDark : Theme.textMid
                                font.pixelSize: 11
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                color: parent.checked ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.1) : "transparent"
                                radius: 10
                                Behavior on color { ColorAnimation { duration: 200 } }
                            }
                        }
                        TabButton {
                            text: "🔍 查找"
                            font.pixelSize: 11
                            contentItem: Text {
                                text: parent.text
                                color: parent.checked ? Theme.textDark : Theme.textMid
                                font.pixelSize: 11
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                color: parent.checked ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.1) : "transparent"
                                radius: 10
                                Behavior on color { ColorAnimation { duration: 200 } }
                            }
                        }
                        TabButton {
                            text: "📂 导入/出"
                            font.pixelSize: 11
                            contentItem: Text {
                                text: parent.text
                                color: parent.checked ? Theme.textDark : Theme.textMid
                                font.pixelSize: 11
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                color: parent.checked ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.1) : "transparent"
                                radius: 10
                                Behavior on color { ColorAnimation { duration: 200 } }
                            }
                        }
                        TabButton {
                            text: "📊 统计"
                            font.pixelSize: 11
                            contentItem: Text {
                                text: parent.text
                                color: parent.checked ? Theme.textDark : Theme.textMid
                                font.pixelSize: 11
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                color: parent.checked ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.1) : "transparent"
                                radius: 10
                                Behavior on color { ColorAnimation { duration: 200 } }
                            }
                        }
                    }

                    // 操作面板 StackLayout
                    StackLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        currentIndex: tabBar.currentIndex

                        // ===== 基础操作 =====
                        ColumnLayout {
                            spacing: 10

                            // 带图标的输入框
                            RowLayout {
                                Text { text: "📖"; font.pixelSize: 14 }
                                TextField {
                                    id: newWordField
                                    Layout.fillWidth: true
                                    placeholderText: "单词"
                                    color: Theme.textDark
                                    font.pixelSize: 12
                                    background: Rectangle {
                                        radius: 12
                                        color: Theme.surface
                                        border.color: Theme.borderInner
                                    }
                                }
                            }
                            RowLayout {
                                Text { text: "📝"; font.pixelSize: 14 }
                                TextField {
                                    id: newTransField
                                    Layout.fillWidth: true
                                    placeholderText: "释义"
                                    color: Theme.textDark
                                    font.pixelSize: 12
                                    background: Rectangle {
                                        radius: 12
                                        color: Theme.surface
                                        border.color: Theme.borderInner
                                    }
                                }
                            }
                            RowLayout {
                                Text { text: "🔊"; font.pixelSize: 14 }
                                TextField {
                                    id: newPhoneticField
                                    Layout.fillWidth: true
                                    placeholderText: "音标"
                                    color: Theme.textDark
                                    font.pixelSize: 12
                                    background: Rectangle {
                                        radius: 12
                                        color: Theme.surface
                                        border.color: Theme.borderInner
                                    }
                                }
                            }

                            // 添加按钮（悬停效果）
                            Button {
                                text: "➕ 添加单词"
                                font.pixelSize: 11
                                Layout.fillWidth: true
                                Layout.preferredHeight: 34
                                background: Rectangle {
                                    radius: 14
                                    color: parent.hovered ? Qt.lighter(Theme.btnPrimaryBg, 1.1) : Theme.btnPrimaryBg
                                    border.color: Theme.btnPrimaryBorder
                                    Behavior on color { ColorAnimation { duration: 150 } }
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: Theme.textDark
                                    font.pixelSize: 11
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: {
                                    var word = newWordField.text.trim().toLowerCase()
                                    if (word === "") { if (rootWindow) rootWindow.statusTextValue = "⚠️ 请输入单词"; return }
                                    for (var i = 0; i < knownWordsModel.count; ++i)
                                        if (knownWordsModel.get(i).word.toLowerCase() === word) { if (rootWindow) rootWindow.statusTextValue = "⚠️ 单词已存在"; return }
                                    root.addWord(word, newTransField.text.trim() || "待补充", newPhoneticField.text.trim())
                                    newWordField.clear(); newTransField.clear(); newPhoneticField.clear()
                                    if (rootWindow) rootWindow.statusTextValue = `✅ 已添加单词: ${word}`
                                }
                            }

                            // 删除选中按钮
                            Button {
                                text: "🗑️ 删除选中"
                                font.pixelSize: 11
                                Layout.fillWidth: true
                                Layout.preferredHeight: 34
                                background: Rectangle {
                                    radius: 14
                                    color: parent.hovered ? Qt.lighter(Theme.btnDangerBg, 1.1) : Theme.btnDangerBg
                                    border.color: Theme.btnDangerBorder
                                    Behavior on color { ColorAnimation { duration: 150 } }
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: Theme.btnDangerText
                                    font.pixelSize: 11
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: {
                                    if (selectedIndices.length === 0) {
                                        if (rootWindow) rootWindow.statusTextValue = "⚠️ 未选中任何单词"
                                        return
                                    }
                                    if (rootWindow) {
                                        var dialog = Qt.createComponent("MessageDialog.qml")
                                        if (dialog.status === Component.Ready) {
                                            var dlg = dialog.createObject(rootWindow)
                                            dlg.dialogTitle = "批量删除"
                                            dlg.dialogText = `确认删除选中的 ${selectedIndices.length} 个单词？`
                                            dlg.standardButtons = "okcancel"
                                            dlg.accepted.connect(function() {
                                                var sorted = selectedIndices.slice().sort((a,b) => b-a)
                                                for (var idx of sorted) root.deleteWord(filteredModel.get(idx).word)
                                                dlg.destroy()
                                            })
                                            dlg.openCentered(rootWindow)
                                        }
                                    } else {
                                        var sorted = selectedIndices.slice().sort((a,b) => b-a)
                                        for (var idx of sorted) root.deleteWord(filteredModel.get(idx).word)
                                    }
                                }
                            }
                        }

                        // ===== 高级查找 =====
                        ColumnLayout {
                            spacing: 10

                            // 搜索框带图标
                            RowLayout {
                                Text { text: "🔍"; font.pixelSize: 14 }
                                TextField {
                                    id: searchField
                                    Layout.fillWidth: true
                                    placeholderText: "输入关键词..."
                                    color: Theme.textDark
                                    font.pixelSize: 12
                                    background: Rectangle {
                                        radius: 12
                                        color: Theme.surface
                                        border.color: Theme.borderInner
                                    }
                                    onTextChanged: rebuildAdvancedFilter()
                                }
                            }

                            // 字段选择
                            ComboBox {
                                id: fieldSelector
                                Layout.fillWidth: true
                                model: ["全部字段", "单词", "释义", "音标"]
                                background: Rectangle {
                                    radius: 12
                                    color: Theme.surface
                                    border.color: Theme.borderInner
                                }
                                contentItem: Text {
                                    text: parent.displayText
                                    color: Theme.textDark
                                    font.pixelSize: 12
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 12
                                }
                            }

                            CheckBox {
                                id: caseSensitive
                                text: "区分大小写"
                                font.pixelSize: 12
                                contentItem: Text {
                                    text: parent.text
                                    color: Theme.textMid
                                    font.pixelSize: 12
                                    verticalAlignment: Text.AlignVCenter
                                }
                                indicator: Rectangle {
                                    implicitWidth: 18; implicitHeight: 18
                                    radius: 4
                                    color: parent.checked ? Theme.btnPrimaryBg : Theme.surface
                                    border.color: Theme.borderInner
                                    Behavior on color { ColorAnimation { duration: 150 } }
                                }
                            }
                            CheckBox {
                                id: exactMatch
                                text: "精确匹配"
                                font.pixelSize: 12
                                contentItem: Text {
                                    text: parent.text
                                    color: Theme.textMid
                                    font.pixelSize: 12
                                    verticalAlignment: Text.AlignVCenter
                                }
                                indicator: Rectangle {
                                    implicitWidth: 18; implicitHeight: 18
                                    radius: 4
                                    color: parent.checked ? Theme.btnPrimaryBg : Theme.surface
                                    border.color: Theme.borderInner
                                    Behavior on color { ColorAnimation { duration: 150 } }
                                }
                            }

                            RowLayout {
                                spacing: 8
                                Button {
                                    text: "应用"
                                    Layout.fillWidth: true
                                    font.pixelSize: 11
                                    background: Rectangle {
                                        radius: 12
                                        color: parent.hovered ? Qt.lighter(Theme.btnPrimaryBg, 1.1) : Theme.btnPrimaryBg
                                        border.color: Theme.btnPrimaryBorder
                                        Behavior on color { ColorAnimation { duration: 150 } }
                                    }
                                    onClicked: rebuildAdvancedFilter()
                                }
                                Button {
                                    text: "清空"
                                    Layout.fillWidth: true
                                    font.pixelSize: 11
                                    background: Rectangle {
                                        radius: 12
                                        color: parent.hovered ? Qt.lighter(Theme.btnNormalBg, 1.1) : Theme.btnNormalBg
                                        border.color: Theme.btnNormalBorder
                                        Behavior on color { ColorAnimation { duration: 150 } }
                                    }
                                    onClicked: searchField.clear()
                                }
                            }
                        }

                        // ===== 导入导出 =====
                        ColumnLayout {
                            spacing: 12

                            Button {
                                text: "📂 导入 CSV"
                                font.pixelSize: 11
                                Layout.fillWidth: true
                                Layout.preferredHeight: 36
                                background: Rectangle {
                                    radius: 12
                                    color: parent.hovered ? Qt.lighter(Theme.btnNormalBg, 1.1) : Theme.btnNormalBg
                                    border.color: Theme.btnNormalBorder
                                    Behavior on color { ColorAnimation { duration: 150 } }
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: Theme.textMid
                                    font.pixelSize: 11
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: {
                                    if (rootWindow) rootWindow.showDemoDialog("导入", "导入 CSV 功能演示（待实现）")
                                }
                            }
                            Button {
                                text: "📤 导出 CSV"
                                font.pixelSize: 11
                                Layout.fillWidth: true
                                Layout.preferredHeight: 36
                                background: Rectangle {
                                    radius: 12
                                    color: parent.hovered ? Qt.lighter(Theme.btnNormalBg, 1.1) : Theme.btnNormalBg
                                    border.color: Theme.btnNormalBorder
                                    Behavior on color { ColorAnimation { duration: 150 } }
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: Theme.textMid
                                    font.pixelSize: 11
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: exportCSV()
                            }
                            Button {
                                text: "📤 导出 JSON"
                                font.pixelSize: 11
                                Layout.fillWidth: true
                                Layout.preferredHeight: 36
                                background: Rectangle {
                                    radius: 12
                                    color: parent.hovered ? Qt.lighter(Theme.btnNormalBg, 1.1) : Theme.btnNormalBg
                                    border.color: Theme.btnNormalBorder
                                    Behavior on color { ColorAnimation { duration: 150 } }
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: Theme.textMid
                                    font.pixelSize: 11
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: exportJSON()
                            }
                        }

                        // ===== 统计 =====
                        ColumnLayout {
                            spacing: 14
                            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                            Text {
                                id: statText
                                text: `📊 总词数: ${knownWordsModel.count}  |  当前显示: ${filteredModel.count}`
                                font.pixelSize: 13
                                color: Theme.textDark
                                horizontalAlignment: Text.AlignHCenter
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                            }

                            Button {
                                text: "🔄 刷新统计"
                                font.pixelSize: 11
                                Layout.fillWidth: true
                                Layout.preferredHeight: 34
                                background: Rectangle {
                                    radius: 14
                                    color: parent.hovered ? Qt.lighter(Theme.btnNormalBg, 1.1) : Theme.btnNormalBg
                                    border.color: Theme.btnNormalBorder
                                    Behavior on color { ColorAnimation { duration: 150 } }
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: Theme.textMid
                                    font.pixelSize: 11
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: updateStats()
                            }
                        }
                    }
                }
            }
        }
    }
}