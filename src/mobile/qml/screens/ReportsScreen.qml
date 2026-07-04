import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Monthly and category breakdown charts using pure QML (Repeater + Rectangle).
// ponytail: bar height calculated from data; upgrade path: Qt Charts or custom canvas
// if fancy animations/legends needed later.
Item {
    id: screen

    property ReportsViewModel model

    ReportsViewModel {
        id: reportsModel
    }

    Component.onCompleted: {
        if (!screen.model)
            screen.model = reportsModel
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingM
        spacing: Theme.spacingM

        Label {
            text: qsTr("Reports")
            font.pixelSize: 16
            font.weight: Font.Bold
            Layout.fillWidth: true
        }

        // Monthly totals chart
        Label {
            text: qsTr("Monthly Totals (Last 12 Months)")
            font.weight: Font.DemiBold
            font.pixelSize: 14
            Layout.fillWidth: true
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 150

            function getMaxValue() {
                let max = 0
                const months = screen.model.monthlyTotals()
                for (let i = 0; i < months.length; i++) {
                    if (months[i].totalMinor > max)
                        max = months[i].totalMinor
                }
                return max > 0 ? max : 1
            }

            Row {
                anchors.fill: parent
                anchors.bottom: parent.bottom
                spacing: 4

                Repeater {
                    model: screen.model.monthlyTotals()

                    Column {
                        width: parent.width / (screen.model.monthlyTotals().length || 1)
                        height: parent.height
                        spacing: 2

                        Rectangle {
                            width: parent.width - 4
                            height: {
                                const max = parent.parent.getMaxValue()
                                return (modelData.totalMinor / max) * (parent.height - 30)
                            }
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: Material.primary

                            ToolTip {
                                visible: mouseArea.containsMouse
                                text: modelData.monthName + ": " + modelData.totalFormatted
                            }

                            MouseArea {
                                id: mouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                            }
                        }

                        Label {
                            text: modelData.month.toLocaleDateString(
                                Qt.locale(AppBackend.localeName), "MMM")
                            font.pixelSize: 10
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }
                }
            }
        }

        // Category breakdown
        Label {
            text: qsTr("Categories (Current Month)")
            font.weight: Font.DemiBold
            font.pixelSize: 14
            Layout.fillWidth: true
            Layout.topMargin: Theme.spacingM
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Column {
                width: parent.width
                spacing: Theme.spacingXS

                Repeater {
                    model: screen.model.categoryTotals(
                        new Date(new Date().getFullYear(), new Date().getMonth(), 1),
                        new Date())

                    Item {
                        width: parent.width
                        height: Theme.spacingL + Theme.spacingXS

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: Theme.spacingM
                            anchors.rightMargin: Theme.spacingM
                            spacing: Theme.spacingM

                            Rectangle {
                                width: 12
                                height: 12
                                radius: 2
                                color: Palette.series(modelData.categoryColor)
                            }

                            Label {
                                text: modelData.categoryName
                                Layout.fillWidth: true
                            }

                            Label {
                                text: modelData.totalFormatted
                                font.weight: Font.DemiBold
                            }
                        }
                    }
                }
            }
        }
    }
}
