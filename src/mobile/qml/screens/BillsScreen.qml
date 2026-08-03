import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Recurring bills, grouped by urgency (overdue, due soon, upcoming, later)
// and headed by what they cost per month.
//
// Tap a row to edit it, long-press for its actions. Editing goes out as a
// signal so the shell can open the sheet at window level, where it overlays
// correctly and the Android back button can find it.
Item {
    id: screen

    property BillController controller
    property BillListModel model

    // Whether paused bills are listed. The screen owns this rather than
    // reading it off the model, because two things drive it — the filter
    // control and pausing a bill — and both have to move the other. A
    // plain binding would not survive the control assigning its own
    // currentIndex on a tap.
    property bool showPaused: false

    signal editRequested(int billId)

    onShowPausedChanged: {
        if (model)
            model.showPaused = showPaused
        scopeControl.currentIndex = showPaused ? 1 : 0
    }

    // The row the action sheet is acting on. Captured by value, not by
    // index: paying or pausing a bill reorders the list underneath it.
    property var _actionRow: null

    function _rowAt(index) {
        return screen.model ? screen.model.get(index) : null
    }

    function openEditFor(index) {
        const row = _rowAt(index)
        if (row && row.billId !== undefined)
            screen.editRequested(row.billId)
    }

    function openActionsFor(index) {
        const row = _rowAt(index)
        if (!row || row.billId === undefined)
            return
        screen._actionRow = row

        const entries = [{ actionId: "edit", text: qsTr("Edit"), icon: "pencil" }]
        if (row.active) {
            entries.push({ actionId: "pay", text: qsTr("Mark paid"),
                           icon: "check-square" })
            entries.push({ actionId: "pause", text: qsTr("Pause"), icon: "pause" })
        } else {
            entries.push({ actionId: "resume", text: qsTr("Resume"), icon: "play" })
        }
        entries.push({ actionId: "delete", text: qsTr("Delete"), icon: "trash",
                       destructive: true })

        actionSheet.openWith(row.name, row.dueLabel, entries)
    }

    function _runAction(actionId) {
        const row = screen._actionRow
        if (!row || !screen.controller)
            return

        switch (actionId) {
        case "edit":
            screen.editRequested(row.billId)
            break
        case "pay":
            screen.payRow(row)
            break
        case "pause":
            screen.setRowActive(row, false)
            break
        case "resume":
            screen.setRowActive(row, true)
            break
        case "delete":
            confirmDelete.open()
            break
        }
    }

    // Logs an expense for the bill and advances its next due date. The
    // amount goes over as exact text rather than a divided float, because
    // the controller re-parses it through the same money parser a typed
    // amount goes through.
    function payRow(row) {
        if (!row || !screen.controller)
            return
        const amountText = AppBackend.amountForEditing(row.amountMinor)
        if (screen.controller.markPaid(row.billId, amountText, new Date(), row.name))
            snackbar.show(qsTr("Logged %1").arg(row.amountFormatted))
        else
            snackbar.show(screen.controller.lastError)
    }

    function setRowActive(row, active) {
        if (!row || !screen.controller)
            return
        if (!screen.controller.setActive(row.billId, active)) {
            snackbar.show(screen.controller.lastError)
            return
        }
        snackbar.show(active ? qsTr("%1 resumed").arg(row.name)
                             : qsTr("%1 paused").arg(row.name))
        // A bill paused while the list is filtered to active bills would
        // just vanish; switching the filter keeps the undo one tap away.
        if (!active)
            screen.showPaused = true
    }

    function _deleteActionRow() {
        const row = screen._actionRow
        if (!row || !screen.controller)
            return
        if (!screen.controller.remove(row.billId))
            snackbar.show(screen.controller.lastError)
    }

    // Keep the model in sync with controller-driven changes even when this
    // screen is used standalone.
    Connections {
        target: screen.controller

        function onBillAdded() { screen.model.refresh() }
        function onBillUpdated() { screen.model.refresh() }
        function onBillRemoved() { screen.model.refresh() }
        function onBillPaid() { screen.model.refresh() }
    }

    // Fallbacks so the screen stands on its own — the shell passes its own
    // shared instances in, which is what keeps every view refreshing
    // together, but the QML tests instantiate this screen by itself.
    BillListModel {
        id: ownModel
    }

    BillController {
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
        heading: qsTr("Delete this bill?")
        message: qsTr("Expenses already logged for it are kept.")
        confirmText: qsTr("Delete")
        destructive: true
        onConfirmed: screen._deleteActionRow()
    }

    Snackbar {
        id: snackbar
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingM
        spacing: Theme.spacingM

        StatCard {
            Layout.fillWidth: true
            title: qsTr("Bills each month")
            value: screen.model ? screen.model.monthlyTotalFormatted : ""
            // Quarterly and yearly bills are counted at their monthly share,
            // which is worth saying out loud rather than leaving the reader
            // to wonder why the numbers do not add up.
            subtitle: screen.model
                ? qsTr("%n active bill(s), spread evenly", "", screen.model.activeCount)
                : ""
        }

        SegmentedControl {
            id: scopeControl

            Layout.fillWidth: true
            options: [qsTr("Active"), qsTr("All")]
            // The other half of the two-way sync in showPaused above. Both
            // sides only fire on a real change, so this settles in one
            // round trip rather than bouncing.
            onCurrentIndexChanged: screen.showPaused = (currentIndex === 1)
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

                section.property: "urgency"
                section.criteria: ViewSection.FullString
                section.delegate: Text {
                    required property string section

                    width: ListView.view.width
                    topPadding: Theme.spacingM
                    bottomPadding: Theme.spacingXs
                    text: {
                        switch (section) {
                        case "overdue":
                            return qsTr("Overdue")
                        case "due":
                            return qsTr("Due soon")
                        case "upcoming":
                            return qsTr("This month")
                        case "paused":
                            return qsTr("Paused")
                        default:
                            return qsTr("Later")
                        }
                    }
                    font.pixelSize: Theme.fontSizeCaption
                    font.weight: Font.DemiBold
                    font.letterSpacing: 0.5
                    color: section === "overdue" ? Theme.critical : Theme.mutedInk
                }

                delegate: BillDelegate {
                    onEditRequested: (index) => screen.openEditFor(index)
                    onMenuRequested: (index) => screen.openActionsFor(index)
                    onPayRequested: (index) => screen.payRow(screen._rowAt(index))
                }
            }

            EmptyState {
                anchors.centerIn: parent
                width: parent.width
                visible: listView.count === 0
                iconName: "bell"
                title: qsTr("No bills yet")
                hint: qsTr("Tap + to track a bill that comes back every month")
            }
        }
    }
}
