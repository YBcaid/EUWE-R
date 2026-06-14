import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "transparent"

    property var rootWindow: null

    signal textChanged(string fullText, int charCount, int wordCount, int lineCount)
    signal extractRequested()
    signal clearRequested()
    signal copyRequested()
    signal helpRequested()
    signal aboutRequested()
    signal importRequested()

    RowLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            Column {
                spacing: 2
                Text {
                    text: "英语生词提取工具"
                    font.pixelSize: 20
                    font.bold: true
                    color: Theme.textDark
                }
                Text {
                    text: "Made By Bcaid"
                    font.pixelSize: 12
                    color: Theme.textLight
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                border.color: Theme.borderInner
                border.width: 1
                radius: 12
                color: Theme.surface

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 10
                    clip: true

                    TextArea {
                        id: mainInput
                        placeholderText: "在此输入文本…"
                        font.pixelSize: 15
                        wrapMode: TextEdit.Wrap
                        background: null
                        color: Theme.textDark
                        selectByMouse: true
                        placeholderTextColor: Theme.textLight

                        onTextChanged: {
                            var text = mainInput.text
                            var chars = text.length
                            var words = text.trim().length === 0 ? 0 : text.trim().split(/\s+/).length
                            var lines = text.length === 0 ? 0 : text.split("\n").length
                            root.textChanged(text, chars, words, lines)
                        }
                    }
                }
            }
        }

        Rectangle {
            width: 210
            Layout.fillHeight: true
            color: "transparent"
            border.color: Theme.borderInner
            border.width: 1
            radius: 12

            Rectangle {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 32
                color: "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "快捷操作"
                    font.pixelSize: 12
                    font.bold: true
                    color: Theme.textMid
                }
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: Theme.borderInner
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                anchors.topMargin: 40
                spacing: 8

                PanelGroup {
                    title: "基础操作"
                }

                CapsuleButton {
                    text: "提取单词"
                    iconType: "play"
                    isPrimary: true
                    onClicked: root.extractRequested()
                }

                CapsuleButton {
                    text: "文件/图片导入"
                    iconType: "upload"
                    onClicked: root.importRequested()
                }

                Item { height: 2 }

                PanelGroup { title: "编辑工具" }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    CapsuleButton {
                        text: "清空"
                        iconType: "trash"
                        isDanger: true
                        Layout.fillWidth: true
                        onClicked: {
                            mainInput.clear()
                            root.clearRequested()
                        }
                    }

                    CapsuleButton {
                        text: "复制"
                        iconType: "copy"
                        Layout.fillWidth: true
                        onClicked: {
                            mainInput.selectAll()
                            mainInput.copy()
                            root.copyRequested()
                        }
                    }
                }

                Item { height: 2 }

                PanelGroup { title: "其他" }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    CapsuleButton {
                        text: "帮助"
                        iconType: "help"
                        Layout.fillWidth: true
                        onClicked: root.helpRequested()
                    }

                    CapsuleButton {
                        text: "关于"
                        iconType: "info"
                        Layout.fillWidth: true
                        onClicked: root.aboutRequested()
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }
    }
}