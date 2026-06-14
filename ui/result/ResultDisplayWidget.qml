import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 850
    height: 550
    minimumWidth: 850
    maximumWidth: 850
    minimumHeight: 550
    maximumHeight: 550
    title: "English Unfamiliar Words Extractor（EUWE）\t V1.1"

    font.family: "Microsoft YaHei"
    font.weight: Font.Normal

    // ==================== 颜色常量 ====================
    readonly property color bgColor: "#f8fafc"
    readonly property color whiteBg: "#ffffff"
    readonly property color borderColor: "#e2e8f0"
    readonly property color textDark: "#2d3748"
    readonly property color textMuted: "#718096"
    readonly property color blueStart: "#4299e1"
    readonly property color blueEnd: "#667eea"
    readonly property color orangeBtn: "#ed8936"
    readonly property color orangeBtnHover: "#dd6b20"
    readonly property color greenBtn: "#48bb78"
    readonly property color greenBtnHover: "#38a169"
    readonly property color purpleBtn: "#9f7aea"
    readonly property color purpleBtnHover: "#805ad5"
    readonly property color grayBtn: "#cbd5e0"
    readonly property color grayBtnHover: "#a0aec0"

    Rectangle {
        anchors.fill: parent
        color: bgColor

        // ========== 顶部标题栏 ==========
        Rectangle {
            id: titleBar
            x: 0; y: 0
            width: 850; height: 70
            color: whiteBg
            border { color: borderColor; width: 1 }

            Rectangle {
                anchors { left: parent.left; right: parent.right; top: parent.bottom }
                height: 3
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: blueStart }
                    GradientStop { position: 0.5; color: purpleBtn }
                    GradientStop { position: 1.0; color: orangeBtn }
                }
            }

            Text {
                x: 20; y: 15
                width: 40; height: 40
                text: "📝"
                font.pixelSize: 30
                renderType: Text.NativeRendering
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                x: 70; y: 15
                text: "生词提取工具 EUWE"
                font.pixelSize: 18
                font.bold: true
                renderType: Text.NativeRendering
                color: textDark
            }
            Text {
                x: 70; y: 40
                text: "Made By Bcaid"
                font.pixelSize: 12
                renderType: Text.NativeRendering
                color: textMuted
            }

            // ---------- 打开熟词库按钮（重构，文字随动）----------
            Item {
                id: openDbWrapper
                x: 700; y: 15
                width: 130; height: 40
                property string hoverState: ""

                // 阴影层
                Rectangle {
                    id: openDbShadow
                    anchors.fill: parent
                    anchors.margins: -3
                    radius: parent.height / 2 + 3
                    color: "#000000"
                    opacity: 0
                    Behavior on opacity {
                        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                    }
                }

                // 按钮内容整体（带变换）
                Item {
                    id: openDbContainer
                    anchors.fill: parent
                    transform: Translate {
                        id: openDbTranslate
                        Behavior on y {
                            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                        }
                    }

                    // 背景矩形 + 两层渐变
                    Rectangle {
                        anchors.fill: parent
                        radius: 20

                        Rectangle {
                            anchors.fill: parent
                            radius: parent.radius
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: blueStart }
                                GradientStop { position: 1.0; color: blueEnd }
                            }
                            opacity: openDbWrapper.hoverState === "hovered" ? 0 : 1
                            Behavior on opacity { NumberAnimation { duration: 150 } }
                        }
                        Rectangle {
                            anchors.fill: parent
                            radius: parent.radius
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "#3182ce" }
                                GradientStop { position: 1.0; color: "#5a67d8" }
                            }
                            opacity: openDbWrapper.hoverState === "hovered" ? 1 : 0
                            Behavior on opacity { NumberAnimation { duration: 150 } }
                        }
                    }

                    // 文字（作为容器子项，随容器移动）
                    Text {
                        anchors.centerIn: parent
                        text: "📚 打开熟词库"
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                        renderType: Text.NativeRendering
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onEntered: {
                        openDbWrapper.hoverState = "hovered"
                        openDbTranslate.y = -2
                        openDbShadow.opacity = 0.08
                    }
                    onExited: {
                        openDbWrapper.hoverState = ""
                        openDbTranslate.y = 0
                        openDbShadow.opacity = 0
                    }
                    onClicked: console.log("打开数据库")
                }
            }
        }

        // ========== 文本区标题 ==========
        Text {
            x: 20; y: 90
            text: "📄 待提取文本"
            font.pixelSize: 14
            font.bold: true
            renderType: Text.NativeRendering
            color: textDark
        }

        // ========== 文本编辑区 ==========
        ScrollView {
            id: scrollView
            x: 20; y: 120
            width: 550; height: 280
            clip: true

            TextArea {
                id: textArea
                anchors.fill: parent
                font.pixelSize: 13
                renderType: Text.NativeRendering      // ← 关键：输入文字变 Widget 风格
                placeholderText: "在此输入文本，或点击下方按钮导入文件/图片..."
                wrapMode: TextEdit.Wrap
                leftPadding: 15
                rightPadding: 10
                topPadding: 12
                bottomPadding: 10

                background: Rectangle {
                    color: "white"
                    border.color: textArea.activeFocus ? blueStart : borderColor
                    border.width: 2
                    radius: 8
                    Behavior on border.color {
                        ColorAnimation { duration: 200; easing.type: Easing.InOutQuad }
                    }
                }

                onTextChanged: updateStats()
            }
        }

        // ========== 右侧功能组 ==========
        Item {
            id: functionGroup
            x: 590; y: 120
            width: 240; height: 280

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border { color: borderColor; width: 2 }
                radius: 10
            }

            Rectangle {
                x: 10; y: -9
                width: 90; height: 20
                color: bgColor
                Text {
                    anchors.centerIn: parent
                    text: "⚡ 快捷操作"
                    font.pixelSize: 14
                    font.bold: true
                    renderType: Text.NativeRendering
                    color: textDark
                }
            }

            ButtonWithShadow {
                x: 15; y: 30
                width: 210; height: 50
                normalColor: orangeBtn
                hoverColor: orangeBtnHover
                text: "📋 左侧文本提取"
                isLeftAlign: true
                onClicked: console.log("提取生词:", textArea.text)
            }

            Text {
                x: 15; y: 90
                width: 210; height: 20
                text: "———— 导入方式 ————"
                color: "#a0aec0"
                font.pixelSize: 11
                renderType: Text.NativeRendering
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            ButtonWithShadow {
                x: 15; y: 115
                width: 210; height: 45
                normalColor: greenBtn
                hoverColor: greenBtnHover
                text: "📁 文件导入 (PDF/DOC/TXT)"
                isLeftAlign: true
                onClicked: console.log("导入文件")
            }

            ButtonWithShadow {
                x: 15; y: 170
                width: 210; height: 45
                normalColor: purpleBtn
                hoverColor: purpleBtnHover
                text: "🖼️ 图片OCR识别"
                isLeftAlign: true
                onClicked: console.log("OCR识别")
            }

            ButtonWithShadow {
                x: 15; y: 225
                width: 100; height: 35
                normalColor: grayBtn
                hoverColor: grayBtnHover
                text: "🗑️ 清空"
                isLeftAlign: false
                textColor: textDark
                onClicked: {
                    textArea.clear()
                    statusLabel.text = "✨ 文本已清空"
                    updateStats()
                }
            }

            ButtonWithShadow {
                x: 125; y: 225
                width: 100; height: 35
                normalColor: grayBtn
                hoverColor: grayBtnHover
                text: "📋 复制"
                isLeftAlign: false
                textColor: textDark
                onClicked: {
                    textArea.selectAll()
                    textArea.copy()
                    statusLabel.text = "✅ 已复制到剪贴板"
                }
            }
        }

        // ========== 底部栏 ==========
        Rectangle {
            id: bottomBar
            x: 0; y: 420
            width: 850; height: 130
            color: whiteBg
            border { color: borderColor; width: 1 }

            ButtonWithShadow {
                x: 20; y: 20
                width: 100; height: 40
                normalColor: blueStart
                hoverColor: "#3182ce"
                text: "❓ 帮助"
                isLeftAlign: false
                radius: 20
                onClicked: console.log("帮助")
            }

            ButtonWithShadow {
                x: 130; y: 20
                width: 100; height: 40
                normalColor: blueEnd
                hoverColor: "#5a67d8"
                text: "ℹ️ 关于"
                isLeftAlign: false
                radius: 20
                onClicked: console.log("关于")
            }

            Rectangle {
                x: 20; y: 70
                width: 810; height: 1
                color: "#e2e8f0"
            }

            Text {
                id: statusLabel
                x: 20; y: 80
                width: 300; height: 30
                text: "✨ 就绪 - 可以开始提取文本"
                color: textMuted
                font.pixelSize: 12
                renderType: Text.NativeRendering
                verticalAlignment: Text.AlignVCenter
            }

            Rectangle {
                x: 550; y: 70
                width: 280; height: 40
                radius: 8
                color: "#f7fafc"
                border.color: borderColor
                Text {
                    id: statsLabel
                    anchors.centerIn: parent
                    text: "📊 字符数: 0 | 单词数: 0 | 行数: 0"
                    color: "#4a5568"
                    font.pixelSize: 12
                    font.bold: true
                    renderType: Text.NativeRendering
                }
            }
        }
    }

    // ==================== 通用按钮组件 ====================
    component ButtonWithShadow : Item {
        id: buttonRoot

        property color normalColor: "gray"
        property color hoverColor: "darkgray"
        property alias text: btnText.text
        property bool isLeftAlign: true
        property color textColor: "white"
        property int radius: 8

        signal clicked()

        Rectangle {
            id: shadow
            anchors.fill: parent
            anchors.margins: -3
            radius: buttonRoot.radius + 3
            color: "#000000"
            opacity: 0
            Behavior on opacity {
                NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
            }
        }

        Rectangle {
            id: buttonRect
            anchors.fill: parent
            radius: buttonRoot.radius
            color: buttonRoot.hovered ? buttonRoot.hoverColor : buttonRoot.normalColor
            Behavior on color {
                ColorAnimation { duration: 180; easing.type: Easing.OutCubic }
            }

            transform: Translate {
                id: btnTranslate
                Behavior on y {
                    NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                }
            }

            Text {
                id: btnText
                anchors {
                    verticalCenter: parent.verticalCenter
                    left: isLeftAlign ? parent.left : undefined
                    horizontalCenter: isLeftAlign ? undefined : parent.horizontalCenter
                    leftMargin: isLeftAlign ? 20 : 0
                }
                color: textColor
                font.pixelSize: 13
                font.bold: true
                renderType: Text.NativeRendering
            }
        }

        property bool hovered: false

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onEntered: {
                buttonRoot.hovered = true
                btnTranslate.y = -1
                shadow.opacity = 0.06
            }
            onExited: {
                buttonRoot.hovered = false
                btnTranslate.y = 0
                shadow.opacity = 0
            }
            onClicked: buttonRoot.clicked()
        }
    }

    function updateStats() {
        var text = textArea.text
        var charCount = 0
        for (var i = 0; i < text.length; i++) {
            if (!/\s/.test(text[i])) charCount++
        }
        var words = text.trim().split(/\s+/).filter(function(w) { return w.length > 0 })
        var wordCount = text.trim() === "" ? 0 : words.length
        var lineCount = text.split(/\n/).length
        if (text === "") lineCount = 0

        statsLabel.text = "📊 字符数: " + charCount + " | 单词数: " + wordCount + " | 行数: " + lineCount
    }

    Component.onCompleted: updateStats()
}