import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: tableRoot
    property ListModel rootModel: ListModel {}
    property string filterText: ""
    property int selectedRow: -1

    signal rowSelected(int index, var itemData)

    color: "transparent"
    clip: true

    property ListModel displayModel: ListModel {}
    property int _filterStamp: rootModel.count + filterText.length
    on_FilterStampChanged: _updateDisplayModel()

    function _updateDisplayModel() {
        displayModel.clear()
        let keyword = filterText.trim().toLowerCase()
        for (let i = 0; i < rootModel.count; ++i) {
            let item = rootModel.get(i)
            if (keyword === "" ||
                item.word.toLowerCase().indexOf(keyword) !== -1 ||
                item.translation.toLowerCase().indexOf(keyword) !== -1) {
                displayModel.append(item)
            }
        }
        if (selectedRow !== -1) {
            selectedRow = -1
            rowSelected(-1, null)
        }
    }

    // 表头
    RowLayout {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 34
        spacing: 1
        z: 2

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredWidth: 140
            height: parent.height
            color: Theme.dark ? "#2A2A3E" : "#F5F0EB"   // 固定色，轻微区分表头
            Text {
                anchors.centerIn: parent
                text: "单词"
                font.pixelSize: 12; font.bold: true
                color: Theme.textMid
            }
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width; height: 1
                color: Theme.borderInner
            }
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredWidth: 280
            height: parent.height
            color: Theme.dark ? "#2A2A3E" : "#F5F0EB"
            Text {
                anchors.centerIn: parent
                text: "释义"
                font.pixelSize: 12; font.bold: true
                color: Theme.textMid
            }
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width; height: 1
                color: Theme.borderInner
            }
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredWidth: 120
            height: parent.height
            color: Theme.dark ? "#2A2A3E" : "#F5F0EB"
            Text {
                anchors.centerIn: parent
                text: "音标"
                font.pixelSize: 12; font.bold: true
                color: Theme.textMid
            }
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width; height: 1
                color: Theme.borderInner
            }
        }
    }

    ScrollView {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: 34
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AsNeeded
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ListView {
            id: listView
            anchors.fill: parent
            model: displayModel
            clip: true
            spacing: 0

            delegate: Rectangle {
                width: listView.width
                height: 38
                color: {
                    if (listView.currentIndex === index)
                        return Theme.dark ? Qt.rgba(45,212,191,0.25) : "#E8DCCC"
                    return (index % 2 === 0) ?
                        (Theme.dark ? Qt.rgba(1,1,1,0.04) : Qt.rgba(0,0,0,0.02)) :
                        "transparent"
                }

                RowLayout {
                    anchors.fill: parent
                    spacing: 1

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 140
                        height: parent.height
                        color: "transparent"
                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            text: model.word
                            color: Theme.textDark
                            font.pixelSize: 13
                            elide: Text.ElideRight
                        }
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 280
                        height: parent.height
                        color: "transparent"
                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            text: model.translation
                            color: Theme.textMid
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 120
                        height: parent.height
                        color: "transparent"
                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            text: model.phonetic || ""
                            color: Theme.textLight
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }
                    }
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width; height: 1
                    color: Theme.borderInner
                    visible: index !== displayModel.count - 1
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    onClicked: {
                        listView.currentIndex = index
                        selectedRow = index
                        rowSelected(index, model)
                    }
                }
            }

            Rectangle {
                visible: displayModel.count === 0
                anchors.centerIn: parent
                width: parent.width; height: 50
                color: "transparent"
                Text {
                    anchors.centerIn: parent
                    text: "暂无数据"
                    color: Theme.textLight
                    font.pixelSize: 12
                }
            }
        }
    }

    onRootModelChanged: _updateDisplayModel()
}