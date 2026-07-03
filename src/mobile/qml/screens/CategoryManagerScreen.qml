import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Category management: add, rename, recolor and delete (business rules —
// system/in-use categories are undeletable — come from the model).
Page {
    id: screen

    property CategoryListModel categories

    signal closed()

    background: Rectangle { color: Theme.surface }

    header: ToolBar {
        Material.background: Theme.cardColor

        RowLayout {
            anchors.fill: parent

            ToolButton {
                icon.source: "../icons/chevron-left.svg"
                onClicked: screen.closed()
            }
            Label {
                Layout.fillWidth: true
                text: qsTr("Categories")
                font.pixelSize: Theme.fontSizeTitle
                color: Theme.primaryInk
            }
            ToolButton {
                icon.source: "../icons/plus.svg"
                onClicked: editDialog.openForAdd()
            }
        }
    }

    Snackbar {
        id: snackbar
    }

    ListView {
        anchors.fill: parent
        anchors.margins: Theme.spacingM
        clip: true
        model: screen.categories
        spacing: Theme.spacingXs

        delegate: ItemDelegate {
            id: row

            required property var model

            width: ListView.view.width
            implicitHeight: Theme.touchTarget + 8

            onClicked: editDialog.openForEdit(model.categoryId, model.name,
                                              model.isSystem)

            contentItem: RowLayout {
                spacing: Theme.spacingM

                Rectangle {
                    width: 16; height: 16; radius: 8
                    color: row.model.color
                }
                Text {
                    Layout.fillWidth: true
                    text: row.model.name
                    font.pixelSize: Theme.fontSizeSubtitle
                    color: Theme.primaryInk
                    elide: Text.ElideRight
                }
                Text {
                    visible: row.model.isSystem
                    text: qsTr("Built-in")
                    font.pixelSize: Theme.fontSizeCaption
                    color: Theme.mutedInk
                }
                ToolButton {
                    visible: !row.model.isSystem && !row.model.inUse
                    icon.source: "../icons/trash.svg"
                    icon.color: Theme.critical
                    onClicked: {
                        if (!screen.categories.remove(row.model.categoryId))
                            snackbar.show(screen.categories.lastError)
                    }
                }
            }
        }
    }

    Dialog {
        id: editDialog

        property int categoryId: 0 // 0 = adding
        property bool systemCategory: false

        function openForAdd() {
            categoryId = 0
            systemCategory = false
            nameField.text = ""
            colorRow.selectedColor = screen.categories.suggestedColor()
            open()
        }

        function openForEdit(id, name, isSystem) {
            categoryId = id
            systemCategory = isSystem
            nameField.text = name
            colorRow.selectedColor = ""
            open()
        }

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(360, parent ? parent.width - 2 * Theme.spacingM : 360)
        title: categoryId === 0 ? qsTr("New category") : qsTr("Edit category")
        modal: true
        standardButtons: Dialog.Save | Dialog.Cancel

        onAccepted: {
            let ok = true
            if (categoryId === 0) {
                ok = screen.categories.add(nameField.text, colorRow.selectedColor)
            } else {
                if (!systemCategory)
                    ok = screen.categories.rename(categoryId, nameField.text)
                if (ok && colorRow.selectedColor !== "")
                    ok = screen.categories.setColor(categoryId,
                                                    colorRow.selectedColor)
            }
            if (!ok)
                snackbar.show(screen.categories.lastError)
        }

        contentItem: ColumnLayout {
            spacing: Theme.spacingM

            TextField {
                id: nameField
                Layout.fillWidth: true
                enabled: !editDialog.systemCategory
                placeholderText: qsTr("Name")
                implicitHeight: Theme.touchTarget
            }

            Flow {
                id: colorRow

                property string selectedColor: ""

                Layout.fillWidth: true
                spacing: Theme.spacingS

                Repeater {
                    model: screen.categories ? screen.categories.paletteColors() : []

                    delegate: Rectangle {
                        required property string modelData

                        width: 36
                        height: 36
                        radius: 18
                        color: modelData
                        border.width: colorRow.selectedColor.toLowerCase()
                                      === modelData.toLowerCase() ? 3 : 0
                        border.color: Theme.primaryInk

                        TapHandler {
                            onTapped: colorRow.selectedColor = modelData
                        }
                    }
                }
            }
        }
    }
}
