import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Searchable, filterable expense list grouped by day, with swipe-to-delete
// and undo.
Item {
    id: screen

    property ExpenseController controller
    property ExpenseListModel model
    property alias searchText: searchField.text

    signal editRequested(int expenseId)

    // Deletes through the controller (so every view refreshes) and offers
    // undo via restore.
    function removeAt(row) {
        const removed = model.get(row)
        if (removed.expenseId === undefined || !controller)
            return
        if (controller.remove(removed.expenseId)) {
            _pendingUndo = removed
            snackbar.show(qsTr("Expense deleted"), qsTr("Undo"))
        }
    }

    function undo() {
        if (!_pendingUndo || !controller)
            return
        controller.restore(_pendingUndo.categoryId, _pendingUndo.amountMinor,
                           _pendingUndo.description, _pendingUndo.date,
                           _pendingUndo.notes)
        _pendingUndo = null
        snackbar.close()
    }

    function requestEditAt(row) {
        const id = model.expenseIdAt(row)
        if (id > 0)
            editRequested(id)
    }

    readonly property bool undoVisible: snackbar.visible
    property var _pendingUndo: null

    // Keep the model in sync with controller-driven changes even when this
    // screen is used standalone.
    Connections {
        target: screen.controller

        function onExpenseAdded() { screen.model.refresh() }
        function onExpenseUpdated() { screen.model.refresh() }
        function onExpenseRemoved() { screen.model.refresh() }
    }

    CategoryListModel {
        id: categoriesModel
    }

    Snackbar {
        id: snackbar
        onActionTriggered: screen.undo()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingM
        spacing: Theme.spacingM

        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: qsTr("Search expenses")
            implicitHeight: Theme.touchTarget
            onTextChanged: screen.model.searchText = text
        }

        CategoryPicker {
            id: filterPicker
            Layout.fillWidth: true
            model: categoriesModel
            allowAll: true
            onSelectedCategoryIdChanged: screen.model.categoryId = selectedCategoryId
        }

        RowLayout {
            Layout.fillWidth: true

            Text {
                Layout.fillWidth: true
                text: qsTr("Total: %1").arg(screen.model ? screen.model.totalFormatted : "")
                font.pixelSize: Theme.fontSizeBody
                color: Theme.secondaryInk
            }
            Text {
                text: screen.model ? qsTr("%n expense(s)", "", screen.model.count) : ""
                font.pixelSize: Theme.fontSizeCaption
                color: Theme.mutedInk
            }
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: screen.model
            spacing: 2

            section.property: "dateSection"
            section.criteria: ViewSection.FullString
            section.delegate: Text {
                required property string section
                width: ListView.view.width
                topPadding: Theme.spacingM
                bottomPadding: Theme.spacingXs
                text: Date.fromLocaleString(Qt.locale(), section, "yyyy-MM-dd")
                    .toLocaleDateString(Qt.locale(AppBackend.localeName),
                                        Locale.LongFormat)
                font.pixelSize: Theme.fontSizeCaption
                font.weight: Font.DemiBold
                color: Theme.mutedInk
            }

            delegate: ExpenseDelegate {
                onEditRequested: (expenseId) => screen.editRequested(expenseId)
                onRemoveRequested: (row) => screen.removeAt(row)
            }

            EmptyState {
                anchors.centerIn: parent
                visible: listView.count === 0
                iconSource: "../icons/receipt.svg"
                title: qsTr("Nothing here")
                hint: qsTr("No expenses match your filters")
            }
        }
    }
}
