import QtQuick

Item {
    id: root
    width: 48
    height: 34

    property string iconType: "home"
    property bool active: false
    signal clicked()

    Rectangle {
        anchors.centerIn: parent
        width: 34
        height: 34
        radius: 6
        visible: active
        color: Theme.dark ? Qt.rgba(45,212,191,0.2) : "#EBE0D5"
        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    Rectangle {
        anchors.centerIn: parent
        width: 34
        height: 34
        radius: 6
        visible: mouseArea.containsMouse && !active
        color: Theme.dark ? Qt.rgba(1,1,1,0.08) : "#F0EAE4"
    }

    readonly property color activeColor: Theme.dark ? Theme.accent : "#4D3E2E"
    readonly property color inactiveColor: Theme.dark ? Theme.textLight : "#A59784"

    readonly property string svgHome: "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'%3E%3Cpath d='M3 9l9-7 9 7v11a2 2 0 0 1-2 2h-5v-8H7v8H5a2 2 0 0 1-2-2z'/%3E%3C/svg%3E"
    readonly property string svgFolder: "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'%3E%3Cpath d='M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z'/%3E%3C/svg%3E"
    readonly property string svgFile: "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'%3E%3Cpath d='M13 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V9z'/%3E%3Cpolyline points='13 2 13 9 20 9'/%3E%3C/svg%3E"

    readonly property string iconSource: {
        var base = iconType === "home" ? svgHome
                 : iconType === "folder" ? svgFolder
                 : svgFile
        var colorHex = (active ? activeColor : inactiveColor).toString().substring(1)
        return base.replace("currentColor", "%23" + colorHex)
    }

    Image {
        anchors.centerIn: parent
        width: 18
        height: 18
        source: iconSource
        sourceSize: Qt.size(20,20)
        smooth: true
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }
}