import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// The price book: what things normally cost, so a shopping run can be
// priced up before it turns into an expense.
//
// Tap a row to edit it, long-press for its actions. Everything that leaves
// this screen goes out as a signal so the shell can open the right sheet —
// the sheets live at window level, where they overlay correctly and where
// the Android back button can find them.
Item {
    id: screen

    property PriceListModel model
    property PriceItemController controller

    signal editRequested(int priceItemId, string previousPrice)
    // Prefill the add-expense sheet from a priced item
    signal logExpenseRequested(int categoryId, string amountText, string description)
    // Prefill the add-item sheet from an existing row, for close variants
    signal duplicateRequested(string name, int categoryId, string unit, string priceText)

    readonly property bool searching: screen.model
        && screen.model.searchText.length > 0

    // The row the action sheet is currently acting on. Captured by value,
    // not by index: the list can be refreshed out from under an open sheet.
    property var _actionRow: null

    function _rowAt(index) {
        return screen.model ? screen.model.get(index) : null
    }

    function openEditFor(index) {
        const row = _rowAt(index)
        if (!row || row.priceItemId === undefined)
            return
        screen.editRequested(row.priceItemId,
                             row.hasPreviousPrice ? row.previousPriceFormatted : "")
    }

    function openActionsFor(index) {
        const row = _rowAt(index)
        if (!row || row.priceItemId === undefined)
            return
        _actionRow = row

        const entries = [
            { actionId: "edit", text: qsTr("Edit"), icon: "pencil" },
            { actionId: "log", text: qsTr("Log as expense"), icon: "cart-plus" },
            { actionId: "duplicate", text: qsTr("Duplicate"), icon: "copy" },
            { actionId: "delete", text: qsTr("Delete"), icon: "trash",
              destructive: true },
        ]
        actionSheet.openWith(row.name, row.priceFormatted, entries)
    }

    function _runAction(actionId) {
        const row = _actionRow
        if (!row)
            return

        const priceText = AppBackend.amountForEditing(row.priceMinor)
        switch (actionId) {
        case "edit":
            screen.editRequested(row.priceItemId,
                                 row.hasPreviousPrice ? row.previousPriceFormatted : "")
            break
        case "log":
            screen.logExpenseRequested(row.hasCategory ? row.categoryId : -1,
                                       priceText, row.name)
            break
        case "duplicate":
            screen.duplicateRequested(qsTr("%1 (copy)").arg(row.name),
                                      row.hasCategory ? row.categoryId : -1,
                                      row.unit, priceText)
            break
        case "delete":
            confirmDelete.open()
            break
        }
    }

    function _deleteActionRow() {
        if (screen._actionRow && screen.controller)
            screen.controller.remove(screen._actionRow.priceItemId)
    }

    // Keep the list in sync with controller-driven changes even when this
    // screen is used standalone.
    Connections {
        target: screen.controller

        function onPriceItemAdded() { screen.model.refresh() }
        function onPriceItemUpdated() { screen.model.refresh() }
        function onPriceItemRemoved() { screen.model.refresh() }
    }

    // Fallbacks so the screen stands on its own — the shell passes its own
    // shared instances in, but the QML tests instantiate this screen alone.
    PriceListModel {
        id: ownModel
    }

    PriceItemController {
        id: ownController
    }

    Component.onCompleted: {
        if (!screen.model)
            screen.model = ownModel
        if (!screen.controller)
            screen.controller = ownController
    }

    ActionSheet {
        id: actionSheet
        onTriggered: (actionId) => screen._runAction(actionId)
    }

    ConfirmDialog {
        id: confirmDelete
        heading: qsTr("Delete this item?")
        message: qsTr("Its price history is deleted with it.")
        confirmText: qsTr("Delete")
        destructive: true
        onConfirmed: screen._deleteActionRow()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingM
        spacing: Theme.spacingM

        SearchField {
            Layout.fillWidth: true
            placeholderText: qsTr("Search the price book")
            onTextChanged: if (screen.model) screen.model.searchText = text
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingM

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                Text {
                    text: screen.model ? qsTr("%n item(s)", "", screen.model.count) : ""
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.secondaryInk
                }

                Text {
                    // Only worth saying when something actually moved.
                    visible: text.length > 0
                    text: {
                        if (!screen.model)
                            return ""
                        const up = screen.model.risenCount
                        const down = screen.model.fallenCount
                        if (up === 0 && down === 0)
                            return ""
                        if (down === 0)
                            return qsTr("%n dearer than before", "", up)
                        if (up === 0)
                            return qsTr("%n cheaper than before", "", down)
                        return qsTr("%1 dearer, %2 cheaper").arg(up).arg(down)
                    }
                    font.pixelSize: Theme.fontSizeCaption
                    color: Theme.mutedInk
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: listView

                anchors.fill: parent
                clip: true
                model: screen.model
                spacing: 2

                delegate: PriceItemDelegate {
                    onEditRequested: (index) => screen.openEditFor(index)
                    onMenuRequested: (index) => screen.openActionsFor(index)
                }
            }

            EmptyState {
                anchors.centerIn: parent
                width: parent.width
                visible: listView.count === 0
                iconName: "basket"
                title: screen.searching ? qsTr("Nothing found")
                                        : qsTr("No prices yet")
                hint: screen.searching
                    ? qsTr("No item matches your search")
                    : qsTr("Tap + to record what something costs")
            }
        }
    }
}
