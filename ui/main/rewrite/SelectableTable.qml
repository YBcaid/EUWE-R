import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property var model
    property var layoutModel
    property var selection

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TableHeader {
            Layout.fillWidth: true
            layoutModel: root.layoutModel
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true

            model: root.model
            clip: true
            cacheBuffer: 200

            delegate: Rectangle {
                width: listView.width
                height: 40

                property string uid: model.uid

                color: selection.isSelected(uid)
                       ? "#3a86ff"
                       : "transparent"

                RowLayout {
                    anchors.fill: parent
                    spacing: 0

                    Text {
                        Layout.preferredWidth: listView.width * layoutModel.word
                        text: model.word
                        color: "white"
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.preferredWidth: listView.width * layoutModel.trans
                        text: model.translation
                        color: "#cccccc"
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.preferredWidth: listView.width * layoutModel.phon
                        text: model.phonetic
                        color: "#aaaaaa"
                    }
                }

                MouseArea {
                    anchors.fill: parent

                    onClicked: function(mouse) {
                        if (mouse.modifiers & Qt.ControlModifier) {
                            selection.toggle(uid)
                        } else if (mouse.modifiers & Qt.ShiftModifier) {
                            selection.range(root.model, uid)
                        } else {
                            selection.single(uid)
                        }
                    }
                }
            }
        }
    }
}