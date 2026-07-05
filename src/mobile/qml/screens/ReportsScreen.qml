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

        RowLayout {
            id: monthlyChart
            Layout.fillWidth: true
            Layout.preferredHeight: 150
            Layout.fillHeight: false // children fill vertically; don't propagate that up
            spacing: 4

            function getMaxValue() {
                let max = 0
                const months = screen.model.monthlyTotals()
                for (let i = 0; i < months.length; i++) {
                    if (months[i].totalMinor > max)
                        max = months[i].totalMinor
                }
                return max > 0 ? max : 1
            }

            Repeater {
                model: screen.model.monthlyTotals()

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1 // equal share regardless of label text width
                    Layout.fillHeight: true
                    spacing: 2

                    // Bar area — bar anchored to the bottom so it grows upward.
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: (modelData.totalMinor / monthlyChart.getMaxValue())
                                    * parent.height
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
                    }

                    Label {
                        Layout.fillWidth: true
                        text: modelData.monthShort
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
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

        ListView {
            objectName: "categoryList"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: Theme.spacingXs
            model: screen.model.categoryTotals(
                new Date(new Date().getFullYear(), new Date().getMonth(), 1),
                new Date())

            delegate: RowLayout {
                width: ListView.view.width
                height: Theme.spacingL + Theme.spacingXs
                spacing: Theme.spacingM

                Rectangle {
                    Layout.leftMargin: Theme.spacingM
                    width: 12
                    height: 12
                    radius: 2
                    color: modelData.categoryColor
                }

                Label {
                    text: modelData.categoryName
                    Layout.fillWidth: true
                }

                Label {
                    text: modelData.totalFormatted
                    font.weight: Font.DemiBold
                    Layout.rightMargin: Theme.spacingM
                }
            }
        }
    }
}
