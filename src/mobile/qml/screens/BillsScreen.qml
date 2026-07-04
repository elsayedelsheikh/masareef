import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Recurring bills organized by urgency (overdue, due soon, upcoming, later).
// Swipe to mark paid; delete removes the bill template.
Item {
    id: screen

    property BillController controller
    property BillListModel model

    // Tapping a row asks the shell to open the edit sheet (which lives at
    // window level so it overlays correctly).
    signal editRequested(int billId)

    // Keep the model in sync with controller-driven changes
    Connections {
        target: screen.controller

        function onBillAdded() { screen.model.refresh() }
        function onBillUpdated() { screen.model.refresh() }
        function onBillRemoved() { screen.model.refresh() }
        function onBillPaid() { screen.model.refresh() }
    }

    BillListModel {
        id: billsModel
    }

    BillController {
        id: billController
    }

    Component.onCompleted: {
        if (!screen.model)
            screen.model = billsModel
        if (!screen.controller)
            screen.controller = billController
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingM
        spacing: Theme.spacingM

        Label {
            text: qsTr("Bills & Reminders")
            font.pixelSize: 16
            font.weight: Font.Bold
            Layout.fillWidth: true
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: screen.model
            clip: true

            section.property: "urgency"
            section.delegate: Rectangle {
                width: ListView.view.width
                height: Theme.spacingM * 2
                color: Material.backgroundColor

                Label {
                    text: {
                        switch (section) {
                        case "overdue":
                            return qsTr("Overdue")
                        case "due":
                            return qsTr("Due Soon (≤7 days)")
                        case "upcoming":
                            return qsTr("Upcoming (≤30 days)")
                        default:
                            return qsTr("Later")
                        }
                    }
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.spacingM
                    font.weight: Font.DemiBold
                }
            }

            delegate: Item {
                width: ListView.view.width
                height: Theme.touchTarget + Theme.spacingXs

                Rectangle {
                    anchors.fill: parent
                    color: Material.backgroundColor

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: Theme.spacingXs
                        spacing: Theme.spacingM

                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: Theme.spacingXs

                            Label {
                                text: model.name
                                font.weight: Font.DemiBold
                                Layout.fillWidth: true
                            }

                            Label {
                                text: model.categoryName + " • " +
                                      model.nextDue.toLocaleDateString()
                                font.pixelSize: 12
                                color: Material.foreground
                                opacity: 0.7
                                Layout.fillWidth: true
                            }
                        }

                        Label {
                            text: model.amountFormatted
                            font.weight: Font.DemiBold
                        }

                        // Logs an expense for this bill and advances its next
                        // due date (per recurrence) via the controller.
                        Button {
                            flat: true
                            text: qsTr("Paid")
                            implicitHeight: Theme.touchTarget
                            Material.foreground: Theme.accent
                            onClicked: {
                                // Convert minor units to plain decimal string for parsing
                                const plainAmount = (model.amountMinor / 100).toFixed(2)
                                screen.controller.markPaid(
                                    model.billId, plainAmount,
                                    new Date(), model.name)
                            }
                        }
                    }

                    // Tap anywhere else on the row to edit; the Paid button
                    // above grabs its own taps so it won't trigger this.
                    TapHandler {
                        onTapped: screen.editRequested(model.billId)
                    }
                }
            }
        }
    }
}
