import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root
    property string text: ""
    property string iconType: ""
    property bool isPrimary: false
    property bool isDanger: false

    signal clicked()

    width: parent.width; height: 34; radius: 17

    color: {
        if (mouseArea.containsMouse) {
            if (isPrimary) return Theme.dark ? "#1FAF9E" : "#E8DCCC"
            if (isDanger) return Theme.dark ? Qt.rgba(255,90,90,0.2) : "#FFE8E8"
            return Theme.dark ? Qt.rgba(1,1,1,0.12) : "#F5F0EB"
        }
        if (isPrimary) return Theme.btnPrimaryBg
        if (isDanger) return Theme.btnDangerBg
        return Theme.btnNormalBg
    }
    border.color: isPrimary ? "transparent" : (isDanger ? Theme.btnDangerBorder : Theme.btnNormalBorder)
    border.width: isPrimary ? 0 : 1

    // 图标数据（与原一致，只需替换 currentColor 的替换逻辑）
    readonly property string svgPlay: "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'%3E%3Cpolygon points='5 3 19 12 5 21 5 3'/%3E%3C/svg%3E"
    readonly property string svgUpload: "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'%3E%3Cpath d='M3 15v4a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-4'/%3E%3Cpolyline points='17 8 12 3 7 8'/%3E%3Cline x1='12' y1='3' x2='12' y2='15'/%3E%3C/svg%3E"
    readonly property string svgTrash: "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'%3E%3Cpolyline points='3 6 5 6 21 6'/%3E%3Cpath d='M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2'/%3E%3Cline x1='10' y1='11' x2='10' y2='17'/%3E%3Cline x1='14' y1='11' x2='14' y2='17'/%3E%3C/svg%3E"
    readonly property string svgCopy: "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'%3E%3Crect x='9' y='9' width='13' height='13' rx='2' ry='2'/%3E%3Cpath d='M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1'/%3E%3C/svg%3E"
    readonly property string svgHelp: "data:image/svg+xml;utf8,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'%3E%3Ccircle cx='12' cy='12' r='10'/%3E%3Cpath d='M9 9a3 3 0 0 1 6 0c0 2-3 2-3 4'/%3E%3Ccircle cx='12' cy='17' r='1'/%3E%3C/svg%3E"
    readonly property string svgInfo: "data:image/svg+xml;utf8,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'%3E%3Ccircle cx='12' cy='12' r='10'/%3E%3Cline x1='12' y1='10' x2='12' y2='16'/%3E%3Ccircle cx='12' cy='7' r='1'/%3E%3C/svg%3E"

    function toHexColor(color) {
        var str = color.toString()
        if (str.startsWith("#")) return str
        var match = str.match(/rgba?\((\d+),\s*(\d+),\s*(\d+)/)
        if (match) {
            var r = parseInt(match[1]).toString(16).padStart(2, '0')
            var g = parseInt(match[2]).toString(16).padStart(2, '0')
            var b = parseInt(match[3]).toString(16).padStart(2, '0')
            return "#" + r + g + b
        }
        return "#000000"
    }

    readonly property color iconColor: {
        if (isDanger) return Theme.btnDangerText
        if (isPrimary) return Theme.dark ? "#0F172A" : "#4D3E2E"
        return Theme.textMid
    }

    readonly property string iconSource: {
        var base = iconType === "play" ? svgPlay :
                  iconType === "upload" ? svgUpload :
                  iconType === "trash" ? svgTrash :
                  iconType === "copy" ? svgCopy :
                  iconType === "help" ? svgHelp :
                  iconType === "info" ? svgInfo : ""
        if (base === "") return ""
        var hexColor = toHexColor(iconColor)
        var colorHex = hexColor.substring(1)
        return base.replace(/currentColor/g, "%23" + colorHex)
    }

    RowLayout {
        anchors.centerIn: parent; spacing: 6
        Image {
            visible: iconType !== ""; width: 16; height: 16
            source: iconSource
            sourceSize: Qt.size(16,16); smooth: true
        }
        Text {
            text: root.text
            font.pixelSize: 12
            font.weight: isPrimary ? Font.DemiBold : Font.Normal
            color: {
                if (isDanger) return Theme.btnDangerText
                if (isPrimary) return Theme.dark ? "#0F172A" : "#4D3E2E"
                return Theme.textMid
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }
}