import QtQuick

QtObject {
    id: root

    property var selectedIds: []
    property string lastSelectedId: ""

    function clear() {
        selectedIds = []
    }

    function isSelected(id) {
        return selectedIds.indexOf(id) !== -1
    }

    function toggle(id) {
        var i = selectedIds.indexOf(id)
        if (i === -1)
            selectedIds = selectedIds.concat([id])
        else {
            var copy = selectedIds.slice()
            copy.splice(i, 1)
            selectedIds = copy
        }
        lastSelectedId = id
    }

    function single(id) {
        selectedIds = [id]
        lastSelectedId = id
    }

    function range(model, id) {
        if (!lastSelectedId) {
            single(id)
            return
        }

        var start = -1
        var end = -1

        for (var i = 0; i < model.count; i++) {
            if (model.get(i).uid === lastSelectedId) start = i
            if (model.get(i).uid === id) end = i
        }

        if (start === -1 || end === -1) return

        if (start > end) {
            var t = start
            start = end
            end = t
        }

        var result = []
        for (var j = start; j <= end; j++)
            result.push(model.get(j).uid)

        selectedIds = result
    }
}