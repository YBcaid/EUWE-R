import QtQuick

Rectangle {
    id: root
    property string text: "—"
    property bool isClose: false
    signal clicked()

    width: 32; height: 26; radius: 4

    color: {
        if (mouseArea.containsMouse) {
            if (isClose) return "#E04343"
            return Theme.btnNormalBg
        }
        return "transparent"
    }

    Text {
        anchors.centerIn: parent
        text: root.text
        font.pixelSize: 16
        color: {
            if (isClose) {
                return mouseArea.containsMouse ? "white" : Theme.textMid
            }
            return Theme.textMid
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }
}