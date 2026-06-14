import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    height: 40
    color: "#2b2b3c"

    property var layoutModel

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // 单词
        Rectangle {
            Layout.preferredWidth: root.width * layoutModel.word
            color: "transparent"

            Text {
                anchors.centerIn: parent
                text: "单词"
                color: "white"
            }

            MouseArea {
                anchors.right: parent.right
                width: 6
                cursorShape: Qt.SizeHorCursor

                property real startX

                onPressed: startX = mouse.x

                onPositionChanged: {
                    var delta = (mouse.x - startX) / root.width
                    layoutModel.setByDrag(delta, "word")
                }
            }
        }

        Rectangle {
            Layout.preferredWidth: root.width * layoutModel.trans
            color: "transparent"

            Text {
                anchors.centerIn: parent
                text: "释义"
                color: "white"
            }

            MouseArea {
                anchors.right: parent.right
                width: 6
                cursorShape: Qt.SizeHorCursor

                property real startX

                onPressed: startX = mouse.x

                onPositionChanged: {
                    var delta = (mouse.x - startX) / root.width
                    layoutModel.setByDrag(delta, "trans")
                }
            }
        }

        Rectangle {
            Layout.preferredWidth: root.width * layoutModel.phon
            color: "transparent"

            Text {
                anchors.centerIn: parent
                text: "音标"
                color: "white"
            }
        }
    }
}