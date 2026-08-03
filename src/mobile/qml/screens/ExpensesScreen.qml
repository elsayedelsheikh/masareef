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

    // The row the action sheet is acting on, captured by value: a refresh
    // can reorder the list while the sheet is open, so every action works
    // from the captured expense id rather than the index it had then.
    property var _actionRow: null

    // Deletes through the controller (so every view refreshes) and offers
    // undo via restore.
    function removeAt(row) {
        _removeCaptured(model ? model.get(row) : null)
    }

    function _removeCaptured(captured) {
        if (!captured || captured.expenseId === undefined || !controller)
            return
        if (controller.remove(captured.expenseId)) {
            _pendingUndo = captured
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

    function openActionsFor(row) {
        const captured = model ? model.get(row) : null
        if (!captured || captured.expenseId === undefined)
            return
        _actionRow = captured

        actionSheet.openWith(
            captured.description.length > 0 ? captured.description
                                            : captured.categoryName,
            captured.amountFormatted + " · " + captured.dateFormatted,
            [
                { actionId: "edit", text: qsTr("Edit"), icon: "pencil" },
                { actionId: "duplicate", text: qsTr("Duplicate"), icon: "copy" },
                { actionId: "select", text: qsTr("Select"), icon: "check-square" },
                { actionId: "delete", text: qsTr("Delete"), icon: "trash",
                  destructive: true },
            ])
    }

    function _runAction(actionId) {
        const captured = _actionRow
        if (!captured || !controller)
            return

        switch (actionId) {
        case "edit":
            editRequested(captured.expenseId)
            break
        case "duplicate":
            // Same expense, dated today — the common case is a repeat of
            // something bought again rather than a copy of that old day.
            if (controller.restore(captured.categoryId, captured.amountMinor,
                                   captured.description, new Date(),
                                   captured.notes) > 0)
                snackbar.show(qsTr("Duplicated"))
            else
                snackbar.show(controller.lastError)
            break
        case "select": {
            // Selection is tracked by row, so the captured expense has to
            // be found again — it may have moved, or gone, since the sheet
            // opened.
            const row = model ? model.rowForExpenseId(captured.expenseId) : -1
            if (row >= 0)
                toggleSelect(row)
            break
        }
        case "delete":
            _removeCaptured(captured)
            break
        }
    }

    // Reassign (not mutate) so QML re-evaluates bindings on selectedRows — a
    // mutated Set fires no change signal, so the Delete button would never show.
    function toggleSelect(row) {
        const next = new Set(selectedRows)
        next.has(row) ? next.delete(row) : next.add(row)
        selectedRows = next
    }

    function removeSelected() {
        const ids = Array.from(selectedRows).map(row => model.expenseIdAt(row))
        if (ids.length > 0 && controller && controller.removeMany(ids))
            selectedRows = new Set()
    }

    function applyDateFilter() {
        screen.model.fromDate = filterFromDate
        screen.model.toDate = filterToDate
    }

    // The repository only date-filters when BOTH bounds are valid, so clearing
    // means making them invalid — not epoch, which would filter to 1970.
    function clearDateFilter() {
        filterFromDate = undefined
        filterToDate = undefined
        screen.model.fromDate = undefined
        screen.model.toDate = undefined
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

    ConfirmDialog {
        id: deleteConfirmDialog
        heading: qsTr("Delete %n expense(s)?", "", screen.selectedRows.size)
        message: qsTr("This cannot be undone.")
        confirmText: qsTr("Delete")
        destructive: true
        onConfirmed: screen.removeSelected()
    }

    ActionSheet {
        id: actionSheet
        onTriggered: (actionId) => screen._runAction(actionId)
    }

    Snackbar {
        id: snackbar
        onActionTriggered: screen.undo()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingM
        spacing: Theme.spacingM

        SearchField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: qsTr("Search expenses")
            onTextChanged: if (screen.model) screen.model.searchText = text
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

            FieldLabel { text: qsTr("From") }

            DateField {
                id: fromDateField
                Layout.fillWidth: true
                value: screen.filterFromDate
                onValueChanged: {
                    screen.filterFromDate = value
                    screen.applyDateFilter()
                }
            }

            FieldLabel { text: qsTr("To") }

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
                onClicked: screen.clearDateFilter()
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

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: listView

                anchors.fill: parent
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
                    // The section key is an ISO date. Formatting it in C++
                    // keeps it in the UI language with Western digits and
                    // gives "Today"/"Yesterday" for the recent days; QML's
                    // toLocaleDateString would use the process locale and
                    // Arabic-Indic numerals that clash with the amounts.
                    // localeName is read so the binding re-runs on a
                    // language switch; it is a dependency, not a condition,
                    // so an empty one must not blank the header.
                    text: {
                        AppBackend.localeName
                        return AppBackend.formatDateSection(section)
                    }
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
                    onMenuRequested: (index) => screen.openActionsFor(index)
                }
            }

            EmptyState {
                anchors.centerIn: parent
                width: parent.width
                visible: listView.count === 0
                iconName: "receipt"
                title: qsTr("Nothing here")
                hint: qsTr("No expenses match your filters")
            }
        }
    }
}
