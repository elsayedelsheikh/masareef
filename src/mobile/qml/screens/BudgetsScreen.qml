import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Overall monthly budget plus per-category budgets with inline editing.
Item {
    id: screen

    property BudgetViewModel budgets

    Snackbar {
        id: snackbar
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingM
        spacing: Theme.spacingM

        Rectangle {
            Layout.fillWidth: true
            radius: Theme.radiusM
            color: Theme.cardColor
            border.color: Theme.gridline
            border.width: 1
            implicitHeight: overallColumn.implicitHeight + 2 * Theme.spacingM

            ColumnLayout {
                id: overallColumn
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    margins: Theme.spacingM
                }
                spacing: Theme.spacingS

                Text {
                    text: qsTr("Monthly budget")
                    font.pixelSize: Theme.fontSizeCaption
                    font.letterSpacing: 0.5
                    color: Theme.mutedInk
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.spacingM

                    TextField {
                        id: overallField
                        Layout.fillWidth: true
                        text: screen.budgets ? screen.budgets.overallBudgetText : ""
                        placeholderText: qsTr("No budget set")
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                        implicitHeight: Theme.touchTarget
                        onAccepted: saveButton.clicked()
                    }
                    Button {
                        id: saveButton
                        icon.source: "../icons/check.svg"
                        highlighted: true
                        implicitHeight: Theme.touchTarget
                        onClicked: {
                            if (!screen.budgets.setOverallBudget(overallField.text))
                                snackbar.show(screen.budgets.lastError)
                        }
                    }
                }

                BudgetGauge {
                    Layout.fillWidth: true
                    visible: screen.budgets && screen.budgets.hasOverallBudget
                    progress: screen.budgets ? screen.budgets.overallProgress : 0
                    overBudget: screen.budgets
                        && screen.budgets.overallSpentMinor
                           > screen.budgets.overallBudgetMinor
                    spentText: screen.budgets
                        ? AppBackend.formatMoney(screen.budgets.overallSpentMinor) : ""
                    budgetText: screen.budgets
                        ? AppBackend.formatMoney(screen.budgets.overallBudgetMinor) : ""
                }
            }
        }

        Text {
            text: qsTr("Category budgets")
            font.pixelSize: Theme.fontSizeCaption
            font.letterSpacing: 0.5
            color: Theme.mutedInk
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: screen.budgets
            spacing: Theme.spacingS

            delegate: ColumnLayout {
                id: row

                required property var model

                width: ListView.view.width
                spacing: Theme.spacingXs

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.spacingM

                    Rectangle {
                        width: 12; height: 12; radius: 6
                        color: row.model.color
                    }
                    Text {
                        Layout.fillWidth: true
                        text: row.model.name
                        font.pixelSize: Theme.fontSizeBody
                        color: Theme.primaryInk
                        elide: Text.ElideRight
                    }
                    Text {
                        visible: row.model.hasBudget
                        text: AppBackend.formatMoney(row.model.spentMinor)
                        font.pixelSize: Theme.fontSizeCaption
                        color: Theme.mutedInk
                    }
                    TextField {
                        Layout.preferredWidth: 110
                        text: row.model.budgetText
                        placeholderText: "—"
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                        horizontalAlignment: TextInput.AlignRight
                        implicitHeight: Theme.touchTarget
                        onEditingFinished: {
                            if (text === row.model.budgetText)
                                return
                            if (!screen.budgets.setCategoryBudget(
                                        row.model.categoryId, text))
                                snackbar.show(screen.budgets.lastError)
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    visible: row.model.hasBudget
                    height: 6
                    radius: 3
                    color: Theme.gridline

                    Rectangle {
                        width: parent.width * row.model.progress
                        height: parent.height
                        radius: 3
                        color: row.model.spentMinor > row.model.budgetMinor
                            ? Theme.critical
                            : row.model.progress >= 0.8 ? Theme.serious : Theme.good
                    }
                }
            }
        }
    }
}
