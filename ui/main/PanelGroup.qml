import QtQuick
import QtQuick.Layouts

Item {
    property string title: ""
    width: parent.width; height: 18

    RowLayout {
        anchors.fill: parent; spacing: 6
        Text {
            text: title; font.pixelSize: 10; font.bold: true
            color: Theme.textLight
        }
        Rectangle {
            Layout.fillWidth: true; height: 1
            color: Theme.borderInner
        }
    }
}