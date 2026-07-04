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

    property var selectedRows: new Set() // Multi-select tracking
    property bool filterExpanded: false
    property date filterFromDate
    property date filterToDate

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

    function toggleSelect(row) {
        if (selectedRows.has(row))
            selectedRows.delete(row)
        else
            selectedRows.add(row)
        listView.forceLayout() // Refresh delegate states
    }

    function removeSelected() {
        const ids = Array.from(selectedRows).map(row => model.expenseIdAt(row))
        if (ids.length > 0 && controller && controller.removeMany(ids)) {
            selectedRows.clear()
            listView.forceLayout()
        }
    }

    function applyDateFilter() {
        screen.model.fromDate = filterFromDate
        screen.model.toDate = filterToDate
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

    Dialog {
        id: deleteConfirmDialog
        title: qsTr("Delete %n expense(s)?", "", selectedRows.size)
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: screen.removeSelected()

        Text {
            text: qsTr("This action cannot be undone.")
            color: Material.foreground
        }
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

        // Collapsible date-range filter
        Button {
            Layout.fillWidth: true
            flat: true
            text: filterExpanded ? qsTr("Hide date filter") : qsTr("Show date filter")
            implicitHeight: Theme.touchTarget
            onClicked: filterExpanded = !filterExpanded
        }

        ColumnLayout {
            Layout.fillWidth: true
            visible: filterExpanded
            spacing: Theme.spacingS

            Text {
                text: qsTr("From")
                font.pixelSize: Theme.fontSizeCaption
                color: Theme.mutedInk
            }

            DateField {
                id: fromDateField
                Layout.fillWidth: true
                value: screen.filterFromDate
                onValueChanged: {
                    screen.filterFromDate = value
                    screen.applyDateFilter()
                }
            }

            Text {
                text: qsTr("To")
                font.pixelSize: Theme.fontSizeCaption
                color: Theme.mutedInk
            }

            DateField {
                id: toDateField
                Layout.fillWidth: true
                value: screen.filterToDate
                onValueChanged: {
                    screen.filterToDate = value
                    screen.applyDateFilter()
                }
            }

            Button {
                Layout.fillWidth: true
                flat: true
                text: qsTr("Clear dates")
                implicitHeight: Theme.touchTarget
                onClicked: {
                    fromDateField.value = new Date(0)
                    toDateField.value = new Date(0)
                    screen.applyDateFilter()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingM

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                Text {
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

            Button {
                visible: selectedRows.size > 0
                flat: true
                text: qsTr("Delete")
                implicitHeight: Theme.touchTarget
                Material.foreground: Material.red
                onClicked: deleteConfirmDialog.open()
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
                isSelected: screen.selectedRows.has(index)
                selectionMode: screen.selectedRows.size > 0
                onEditRequested: (expenseId) => screen.editRequested(expenseId)
                onRemoveRequested: (row) => screen.removeAt(row)
                onSelectionToggled: (index) => screen.toggleSelect(index)
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
