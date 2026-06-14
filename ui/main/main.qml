import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Window {
    id: rootWindow
    width: 853
    height: 583
    visible: true
    title: "EUWE V1.3"
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.Window | Qt.NoDropShadowWindowHint

    property Theme theme: Theme

    function toggleTheme() {
        Theme.dark = !Theme.dark
    }

    property string statusTextValue: "就绪 - 可以开始提取文本"
    property int currentView: 0   // 0:提取页, 1:结果页, 2:管理页

    function showDemoDialog(title, text) {
        var component = Qt.createComponent("MessageDialog.qml")
        if (component.status === Component.Ready) {
            var dialog = component.createObject(rootWindow)
            dialog.dialogTitle = title
            dialog.dialogText = text
            dialog.standardButtons = "ok"
            dialog.openCentered(rootWindow)
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        radius: 16
        border.color: Theme.border
        border.width: 1
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 1
        radius: 15
        color: Theme.bg
        clip: true

        Rectangle {
            id: titleBar
            height: 48
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            topLeftRadius: 15
            topRightRadius: 15
            color: Theme.titleBg

            MouseArea {
                anchors.fill: parent
                property point clickPos
                onPressed: clickPos = Qt.point(mouseX, mouseY)
                onPositionChanged: if(pressed) { rootWindow.x += mouseX - clickPos.x; rootWindow.y += mouseY - clickPos.y }
                onDoubleClicked: rootWindow.visibility === Window.Maximized ? rootWindow.showNormal() : rootWindow.showMaximized()
            }

            Row {
                anchors.left: parent.left
                anchors.leftMargin: 24
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8
                Image {
                    width: 20; height: 20
                    source: "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='24' height='24' viewBox='0 0 24 24' fill='none' stroke='"+(Theme.dark?"%23F1F1F6":"%232C2C2C")+"' stroke-width='2'%3E%3Cpath d='M16 3h5v5M14 9l6-6M4 20h16a2 2 0 0 0 2-2V6a2 2 0 0 0-2-2h-5'/%3E%3Cpath d='M12 12v6'/%3E%3Cpath d='M9 15h6'/%3E%3C/svg%3E"
                    sourceSize: Qt.size(20,20); smooth: true
                }
                Text {
                    text: "EUWE V1.3"
                    font.pixelSize: 16
                    font.bold: true
                    color: Theme.textDark
                }
            }

            RowLayout {
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8
                ThemeToggleBtn { onToggleTheme: rootWindow.toggleTheme() }
                WindowControlBtn { text: "—"; onClicked: rootWindow.showMinimized() }
                WindowControlBtn { text: "☐"; onClicked: rootWindow.visibility===Window.Maximized ? rootWindow.showNormal() : rootWindow.showMaximized() }
                WindowControlBtn { text: "✕"; isClose: true; onClicked: rootWindow.close() }
            }
        }

        Rectangle {
            id: statusBar
            height: 34
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            color: "transparent"

            Rectangle {
                width: parent.width
                height: 1
                anchors.top: parent.top
                color: Theme.borderInner
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                spacing: 16

                Text {
                    id: statusText
                    text: rootWindow.statusTextValue
                    font.pixelSize: 11
                    color: Theme.textLight
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }

                Row {
                    spacing: 16
                    visible: currentView === 0
                    Row { spacing:4; Text { text:"字符:"; font.pixelSize:11; color:Theme.textLight } Text { id:charValue; text:"0"; font.pixelSize:11; font.bold:true; color:Theme.textMid } }
                    Row { spacing:4; Text { text:"单词:"; font.pixelSize:11; color:Theme.textLight } Text { id:wordValue; text:"0"; font.pixelSize:11; font.bold:true; color:Theme.textMid } }
                    Row { spacing:4; Text { text:"行:"; font.pixelSize:11; color:Theme.textLight } Text { id:lineValue; text:"0"; font.pixelSize:11; font.bold:true; color:Theme.textMid } }
                }

                Row {
                    spacing: 12
                    visible: currentView === 1
                    Row { spacing:4; Text { text:"未识别:"; font.pixelSize:11; color:Theme.textLight } Text { text: unknownModel.count; font.pixelSize:11; font.bold:true; color: Theme.btnDangerText } }
                    Row { spacing:4; Text { text:"生词:"; font.pixelSize:11; color:Theme.textLight } Text { text: newWordModel.count; font.pixelSize:11; font.bold:true; color: Theme.accent } }
                    Row { spacing:4; Text { text:"熟词:"; font.pixelSize:11; color:Theme.textLight } Text { text: knownWordModel.count; font.pixelSize:11; font.bold:true; color: "#28a745" } }
                }

                Row {
                    spacing: 16
                    visible: currentView === 2
                }
            }
        }

        Row {
            anchors.top: titleBar.bottom
            anchors.bottom: statusBar.top
            anchors.left: parent.left
            anchors.right: parent.right

            Rectangle {
                id: sidebar
                width: 48
                height: parent.height
                color: Theme.sidebarBg

                Rectangle {
                    width: 1
                    height: parent.height
                    anchors.right: parent.right
                    color: Theme.borderInner
                }

                Rectangle {
                    id: activeIndicator
                    width: 34; height: 34; radius: 6
                    color: Theme.dark ? Qt.rgba(45,212,191,0.2) : "#EBE0D5"
                    x: (sidebar.width - width)/2
                    y: {
                        var target = [sideBar0, sideBar1, sideBar2][currentView]
                        return target ? target.mapToItem(sidebar, 0, 0).y : 0
                    }
                    Behavior on y { NumberAnimation { duration: 200 } }
                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                ColumnLayout {
                    anchors.top: parent.top
                    width: parent.width
                    spacing: 12

                    SideBarItem {
                        id: sideBar0
                        iconType: "home"
                        active: currentView===0
                        onClicked: currentView = 0
                    }
                    SideBarItem {
                        id: sideBar1
                        iconType: "folder"
                        active: currentView===1
                        onClicked: currentView = 1
                    }
                    SideBarItem {
                        id: sideBar2
                        iconType: "file"
                        active: currentView===2
                        onClicked: currentView = 2
                    }
                }
            }

            Rectangle {
                width: parent.width - sidebar.width
                height: parent.height
                color: "transparent"

                StackLayout {
                    anchors.fill: parent
                    currentIndex: currentView

                    ExtractPage {
                        onTextChanged: { charValue.text = charCount; wordValue.text = wordCount; lineValue.text = lineCount; statusTextValue = "✨ 已更新" }
                        onExtractRequested: { statusTextValue = "提取完成 (演示)" }
                        onClearRequested: { charValue.text = "0"; wordValue.text = "0"; lineValue.text = "0"; statusTextValue = "✨ 文本已清空" }
                        onCopyRequested: { statusTextValue = "已复制到剪贴板" }
                        onHelpRequested: showDemoDialog("帮助", "联系 Bcaid\nQQ:75011847\nWX:YBcaid")
                        onAboutRequested: showDemoDialog("关于", "EUWE 生词提取工具 V1.3\n演示版")
                        onImportRequested: statusTextValue = "导入功能待集成 (可连接C++后端)"
                    }

                    // ★ 结果页：不再连接任何信号，数据操作完全由 ResultPage 内部完成
                    ResultPage {
                        unknownModel: unknownModel
                        newWordModel: newWordModel
                        knownWordModel: knownWordModel
                        mainWindow: rootWindow
                    }

                    WordManagePage {
                        knownWordsModel: knownWordModel
                        rootWindow: rootWindow
                        onAddWord: { (word, translation, phonetic) => {
                            knownWordModel.append({ word: word, translation: translation, phonetic: phonetic })
                            statusTextValue = `已添加单词: ${word}`
                        }}
                        onDeleteWord: { (word) => {
                            for (var i = 0; i < knownWordModel.count; ++i) {
                                if (knownWordModel.get(i).word === word) {
                                    knownWordModel.remove(i)
                                    statusTextValue = `️已删除单词: ${word}`
                                    break
                                }
                            }
                        }}
                    }
                }
            }
        }
    }

    // 全局数据模型
    ListModel { id: unknownModel }
    ListModel { id: newWordModel }
    ListModel { id: knownWordModel }

    Component.onCompleted: {
        // ===== 未识别（15个） =====
        unknownModel.append({ word: "algorithm", translation: "", phonetic: "" })
        unknownModel.append({ word: "synthesize", translation: "", phonetic: "" })
        unknownModel.append({ word: "ubiquitous", translation: "", phonetic: "" })
        unknownModel.append({ word: "paradigm", translation: "", phonetic: "" })
        unknownModel.append({ word: "ephemeral", translation: "", phonetic: "" })
        unknownModel.append({ word: "serendipity", translation: "", phonetic: "" })
        unknownModel.append({ word: "ambiguous", translation: "", phonetic: "" })
        unknownModel.append({ word: "benevolent", translation: "", phonetic: "" })
        unknownModel.append({ word: "cacophony", translation: "", phonetic: "" })
        unknownModel.append({ word: "dichotomy", translation: "", phonetic: "" })
        unknownModel.append({ word: "euphoria", translation: "", phonetic: "" })
        unknownModel.append({ word: "facetious", translation: "", phonetic: "" })
        unknownModel.append({ word: "garrulous", translation: "", phonetic: "" })
        unknownModel.append({ word: "harbinger", translation: "", phonetic: "" })
        unknownModel.append({ word: "iconoclast", translation: "", phonetic: "" })

        // ===== 生词（15个） =====
        newWordModel.append({ word: "apple", translation: "苹果", phonetic: "/ˈæp.əl/" })
        newWordModel.append({ word: "car", translation: "汽车", phonetic: "/kɑːr/" })
        newWordModel.append({ word: "house", translation: "房子", phonetic: "/haʊs/" })
        newWordModel.append({ word: "computer", translation: "计算机", phonetic: "/kəmˈpjuː.t̬ɚ/" })
        newWordModel.append({ word: "beautiful", translation: "美丽的", phonetic: "/ˈbjuː.t̬ə.fəl/" })
        newWordModel.append({ word: "knowledge", translation: "知识", phonetic: "/ˈnɑː.lɪdʒ/" })
        newWordModel.append({ word: "environment", translation: "环境", phonetic: "/ɪnˈvaɪ.rən.mənt/" })
        newWordModel.append({ word: "communication", translation: "交流", phonetic: "/kəˌmjuː.nɪˈkeɪ.ʃən/" })
        newWordModel.append({ word: "experience", translation: "经验", phonetic: "/ɪkˈspɪr.i.əns/" })
        newWordModel.append({ word: "information", translation: "信息", phonetic: "/ˌɪn.fɚˈmeɪ.ʃən/" })
        newWordModel.append({ word: "language", translation: "语言", phonetic: "/ˈlæŋ.ɡwɪdʒ/" })
        newWordModel.append({ word: "opportunity", translation: "机会", phonetic: "/ˌɑː.pɚˈtuː.nə.t̬i/" })
        newWordModel.append({ word: "technology", translation: "技术", phonetic: "/tekˈnɑː.lə.dʒi/" })
        newWordModel.append({ word: "university", translation: "大学", phonetic: "/ˌjuː.nɪˈvɝː.sə.t̬i/" })
        newWordModel.append({ word: "vocabulary", translation: "词汇", phonetic: "/voʊˈkæb.jə.ler.i/" })

        // ===== 熟词（15个） =====
        knownWordModel.append({ word: "hello", translation: "你好", phonetic: "/həˈloʊ/" })
        knownWordModel.append({ word: "world", translation: "世界", phonetic: "/wɝːld/" })
        knownWordModel.append({ word: "good", translation: "好的", phonetic: "/ɡʊd/" })
        knownWordModel.append({ word: "morning", translation: "早晨", phonetic: "/ˈmɔːr.nɪŋ/" })
        knownWordModel.append({ word: "thank", translation: "感谢", phonetic: "/θæŋk/" })
        knownWordModel.append({ word: "friend", translation: "朋友", phonetic: "/frend/" })
        knownWordModel.append({ word: "family", translation: "家庭", phonetic: "/ˈfæm.əl.i/" })
        knownWordModel.append({ word: "love", translation: "爱", phonetic: "/lʌv/" })
        knownWordModel.append({ word: "happy", translation: "快乐的", phonetic: "/ˈhæp.i/" })
        knownWordModel.append({ word: "sad", translation: "悲伤的", phonetic: "/sæd/" })
        knownWordModel.append({ word: "big", translation: "大的", phonetic: "/bɪɡ/" })
        knownWordModel.append({ word: "small", translation: "小的", phonetic: "/smɑːl/" })
        knownWordModel.append({ word: "hot", translation: "热的", phonetic: "/hɑːt/" })
        knownWordModel.append({ word: "cold", translation: "冷的", phonetic: "/koʊld/" })
        knownWordModel.append({ word: "beautiful", translation: "美丽的", phonetic: "/ˈbjuː.t̬ə.fəl/" })
    }
}