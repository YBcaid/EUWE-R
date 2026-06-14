import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: control
    modal: true
    focus: true
    padding: 0

    property var mainWindow: null
    property string dialogTitle: ""
    property string dialogText: ""
    property string standardButtons: "ok"
    property string type: "info"
    property var onAccepted: function() {}
    property var onRejected: function() {}
    property real targetY: 0

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property color accentColor: {
        switch (type) {
        case "success": return Theme.btnPrimaryBg
        case "warning": return "#E6A23C"
        case "error":   return "#D96C6C"
        default:        return Theme.btnPrimaryBg
        }
    }

    Overlay.modal: Rectangle {
        radius: 16
        color: Qt.rgba(0, 0, 0, 0.28)
    }

    background: Rectangle {
        radius: 16
        color: Theme.surface
        border.width: 0.5
        border.color: control.accentColor
    }

    implicitWidth: Math.min(400, textItem.implicitWidth + 60)
    implicitHeight: layout.implicitHeight + 36

    contentItem: ColumnLayout {
        id: layout
        spacing: 14
        anchors.fill: parent
        anchors.margins: 20

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                visible: dialogTitle !== ""
                text: dialogTitle
                font.pixelSize: 17
                font.bold: true
                color: Theme.textDark
            }

            Rectangle {
                visible: dialogTitle !== ""
                Layout.fillWidth: true
                height: 1
                color: Theme.borderInner
            }
        }

        Text {
            id: textItem
            text: dialogText
            wrapMode: Text.WordWrap
            font.pixelSize: 14
            color: Theme.textMid
            lineHeight: 1.4
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 0
            Layout.bottomMargin: 35
            Layout.alignment: Qt.AlignRight
            spacing: 10

            Repeater {
                model: {
                    switch (standardButtons) {
                    case "ok": return ["确定"]
                    case "okcancel": return ["取消", "确定"]
                    case "yesno": return ["否", "是"]
                    default: return ["确定"]
                    }
                }

                delegate: Rectangle {
                    radius: 18
                    implicitWidth: 78
                    implicitHeight: 34
                    property bool primary: (modelData === "确定" || modelData === "是")

                    color: primary ? Theme.btnPrimaryBg : Theme.btnNormalBg
                    border.width: primary ? 0 : 1
                    border.color: primary ? "transparent" : Theme.btnNormalBorder

                    Behavior on color { ColorAnimation { duration: 120 } }

                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        font.pixelSize: 13
                        color: Theme.textDark
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor

                        onEntered: {
                            if (primary)
                                parent.color = Theme.btnPrimaryBorder
                            else
                                parent.color = Theme.btnNormalBorder
                        }

                        onExited: {
                            parent.color = primary ? Theme.btnPrimaryBg : Theme.btnNormalBg
                        }

                        onClicked: {
                            if (primary) {
                                control.onAccepted()
                                control.close()
                            } else {
                                control.onRejected()
                                control.close()
                            }
                        }
                    }
                }
            }
        }
    }

    enter: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 160 }
            NumberAnimation { property: "scale"; from: 0.92; to: 1; duration: 220; easing.type: Easing.OutBack; easing.overshoot: 1.4 }
            NumberAnimation { property: "y"; from: control.targetY + 20; to: control.targetY; duration: 240; easing.type: Easing.OutBack; easing.overshoot: 1.2 }
        }
    }

    exit: Transition {
        ParallelAnimation {
            NumberAnimation { property: "opacity"; to: 0; duration: 180 }
            NumberAnimation { property: "scale"; to: 0.88; duration: 180; easing.type: Easing.InOutQuad }
            NumberAnimation { property: "y"; to: control.targetY + 25; duration: 180; easing.type: Easing.InOutQuad }
        }
    }

    Keys.onEscapePressed: {
        control.onRejected()
        control.close()
    }

    function openCentered(win) {
        if (!win) return
        mainWindow = win
        targetY = (win.height - height) * 0.3
        x = (win.width - width) / 2
        y = targetY + 20
        open()
    }
}