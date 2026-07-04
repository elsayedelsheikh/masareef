import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Category management: add, rename, recolor and delete. Deleting an
// in-use category opens a reassign dialog instead of failing outright.
Page {
    id: screen

    property CategoryListModel categories
    property alias reassignDialog: reassignDialog
    property alias reassignPicker: picker

    signal closed()

    function requestDelete(categoryId, name, inUse) {
        if (inUse)
            reassignDialog.openFor(categoryId, name)
        else if (!screen.categories.remove(categoryId))
            snackbar.show(screen.categories.lastError)
    }

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

            onClicked: editDialog.openForEdit(model.categoryId, model.name)

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
                ToolButton {
                    visible: screen.categories.count > 1
                    icon.source: "../icons/trash.svg"
                    icon.color: Theme.critical
                    onClicked: screen.requestDelete(row.model.categoryId, row.model.name,
                                                    row.model.inUse)
                }
            }
        }
    }

    Dialog {
        id: editDialog

        property int categoryId: 0 // 0 = adding

        function openForAdd() {
            categoryId = 0
            nameField.text = ""
            colorRow.selectedColor = screen.categories.suggestedColor()
            open()
        }

        function openForEdit(id, name) {
            categoryId = id
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

    Dialog {
        id: reassignDialog

        property int categoryId: 0
        property string categoryName: ""

        function openFor(id, name) {
            categoryId = id
            categoryName = name
            picker.selectedCategoryId = -1
            open()
        }

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(360, parent ? parent.width - 2 * Theme.spacingM : 360)
        title: qsTr("Move expenses from \"%1\"").arg(reassignDialog.categoryName)
        modal: true
        standardButtons: Dialog.Save | Dialog.Cancel

        // Save stays disabled until a destination is chosen, so the user can't
        // trigger a confusing "Category not found." error by saving nothing.
        onOpened: {
            const save = standardButton(Dialog.Save)
            if (save)
                save.enabled = Qt.binding(() => picker.selectedCategoryId >= 0)
        }

        onAccepted: {
            if (!screen.categories.removeAndReassign(reassignDialog.categoryId,
                                                      picker.selectedCategoryId))
                snackbar.show(screen.categories.lastError)
        }

        contentItem: ColumnLayout {
            spacing: Theme.spacingM

            Text {
                Layout.fillWidth: true
                text: qsTr("This category is used by existing expenses or recurring bills. Choose where to move them:")
                wrapMode: Text.WordWrap
                color: Theme.mutedInk
                font.pixelSize: Theme.fontSizeBody
            }

            CategoryPicker {
                id: picker
                Layout.fillWidth: true
                model: screen.categories
                excludeCategoryId: reassignDialog.categoryId
            }
        }
    }
}
