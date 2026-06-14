import QtQuick
import QtQuick.Layouts

RowLayout {
    id: root
    spacing: 0

    property var layoutModel
    property real totalWidth

    function colWidth(ratio) {
        return totalWidth * ratio
    }
}