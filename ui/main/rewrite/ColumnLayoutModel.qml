import QtQuick

QtObject {
    id: root

    property real word: 0.4
    property real trans: 0.4
    property real phon: 0.2

    function normalize() {
        var sum = word + trans + phon
        if (sum <= 0) return
        word /= sum
        trans /= sum
        phon /= sum
    }

    function setByDrag(delta, column) {
        var min = 0.1

        if (column === "word") {
            word += delta
            trans -= delta
        } else if (column === "trans") {
            trans += delta
            phon -= delta
        }

        word = Math.max(min, word)
        trans = Math.max(min, trans)
        phon = Math.max(min, phon)

        normalize()
    }
}