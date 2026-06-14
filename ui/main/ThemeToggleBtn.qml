import QtQuick

Rectangle {
    id: root
    width: 32; height: 26; radius: 4

    // 不再需要 themeIndex 属性，改用 Theme.dark
    signal toggleTheme()

    color: {
        if (mouseArea.containsMouse) {
            return Theme.dark ? Qt.rgba(1,1,1,0.15) : Qt.rgba(0,0,0,0.08)
        }
        return "transparent"
    }

    Image {
        anchors.centerIn: parent; width: 18; height: 18
        // 根据 Theme.dark 选择对应的 SVG（内嵌数据与原代码一致，只替换了颜色判断）
        source: Theme.dark ?
            "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='24' height='24' viewBox='0 0 24 24' fill='none' stroke='%238080a0' stroke-width='2' stroke-linecap='round' stroke-linejoin='round'%3E%3Cpath d='M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z'/%3E%3C/svg%3E" :
            "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='24' height='24' viewBox='0 0 24 24' fill='none' stroke='%23666' stroke-width='2' stroke-linecap='round' stroke-linejoin='round'%3E%3Ccircle cx='12' cy='12' r='5'/%3E%3Cline x1='12' y1='1' x2='12' y2='3'/%3E%3Cline x1='12' y1='21' x2='12' y2='23'/%3E%3Cline x1='4.22' y1='4.22' x2='5.64' y2='5.64'/%3E%3Cline x1='18.36' y1='18.36' x2='19.78' y2='19.78'/%3E%3Cline x1='1' y1='12' x2='3' y2='12'/%3E%3Cline x1='21' y1='12' x2='23' y2='12'/%3E%3Cline x1='4.22' y1='19.78' x2='5.64' y2='18.36'/%3E%3Cline x1='18.36' y1='5.64' x2='19.78' y2='4.22'/%3E%3C/svg%3E"
        sourceSize: Qt.size(20,20); smooth: true
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.toggleTheme()
    }
}