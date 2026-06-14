import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    ListModel {
        id: wordModel

        ListElement { uid: "1"; word: "apple"; translation: "苹果"; phonetic: "/ˈæpəl/"; type: "unknown" }
        ListElement { uid: "2"; word: "book"; translation: "书"; phonetic: "/bʊk/"; type: "known" }
        ListElement { uid: "3"; word: "run"; translation: "跑"; phonetic: "/rʌn/"; type: "new" }
    }

    function filter(type) {
        var arr = []
        for (var i = 0; i < wordModel.count; i++) {
            var it = wordModel.get(i)
            if (it.type === type)
                arr.push(it)
        }
        return arr
    }

    ColumnLayout {
        anchors.fill: parent

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Repeater {
                model: ["unknown", "new", "known"]

                delegate: Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#1e1e2e"

                    ColumnLayout {
                        anchors.fill: parent

                        Text {
                            text: modelData
                            color: "white"
                        }

                        SelectableTable {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            model: filter(modelData)

                            layoutModel: ColumnLayoutModel {}
                            selection: SelectionController {}
                        }
                    }
                }
            }
        }
    }
}