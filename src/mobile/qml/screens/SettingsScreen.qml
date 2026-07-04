import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Theme, language, currency, category management, backup/restore and About.
Flickable {
    id: screen

    signal manageCategoriesRequested()

    contentWidth: width
    contentHeight: column.implicitHeight + 2 * Theme.spacingM
    clip: true

    Dialog {
        id: restoreConfirmDialog
        property string backupPath: ""

        title: qsTr("Restore from backup?")
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: {
            if (AppBackend.restore(backupPath)) {
                // ponytail: assuming modelsRefreshNeeded is connected at app level
                showMessage.text = qsTr("Backup restored successfully")
                showMessage.open()
            } else {
                showMessage.text = qsTr("Restore failed")
                showMessage.open()
            }
        }

        Text {
            text: qsTr("This will replace your current data with the selected backup. Continue?")
            color: Material.foreground
        }
    }

    Dialog {
        id: showMessage
        title: qsTr("Backup")
        property alias text: messageText.text
        standardButtons: Dialog.Ok

        Text {
            id: messageText
            color: Material.foreground
        }
    }

    component SectionLabel: Text {
        font.pixelSize: Theme.fontSizeCaption
        font.letterSpacing: 0.5
        color: Theme.mutedInk
    }

    ColumnLayout {
        id: column
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: Theme.spacingM
        }
        spacing: Theme.spacingS

        SectionLabel { text: qsTr("Appearance") }

        RowLayout {
            Layout.fillWidth: true

            RadioButton {
                text: qsTr("System")
                checked: ThemeController.preference === "system"
                onClicked: ThemeController.preference = "system"
            }
            RadioButton {
                text: qsTr("Light")
                checked: ThemeController.preference === "light"
                onClicked: ThemeController.preference = "light"
            }
            RadioButton {
                text: qsTr("Dark")
                checked: ThemeController.preference === "dark"
                onClicked: ThemeController.preference = "dark"
            }
        }

        SectionLabel {
            Layout.topMargin: Theme.spacingM
            text: qsTr("Language")
        }

        RowLayout {
            Layout.fillWidth: true

            RadioButton {
                text: qsTr("System")
                checked: AppBackend.language === "system"
                onClicked: AppBackend.language = "system"
            }
            RadioButton {
                text: "English"
                checked: AppBackend.language === "en"
                onClicked: AppBackend.language = "en"
            }
            RadioButton {
                text: "العربية"
                checked: AppBackend.language === "ar"
                onClicked: AppBackend.language = "ar"
            }
        }

        SectionLabel {
            Layout.topMargin: Theme.spacingM
            text: qsTr("Currency")
        }

        TextField {
            id: currencyField
            Layout.preferredWidth: 140
            text: AppBackend.currencyCode
            maximumLength: 3
            font.capitalization: Font.AllUppercase
            implicitHeight: Theme.touchTarget
            onEditingFinished: {
                const code = text.toUpperCase().trim()
                if (code.length === 3)
                    AppBackend.currencyCode = code
                else
                    text = AppBackend.currencyCode
            }
        }

        SectionLabel {
            Layout.topMargin: Theme.spacingM
            text: qsTr("Backup & Restore")
        }

        Button {
            Layout.fillWidth: true
            text: qsTr("Back up now")
            implicitHeight: Theme.touchTarget
            onClicked: {
                if (AppBackend.backupNow())
                    showMessage.text = qsTr("Backup created successfully")
                else
                    showMessage.text = qsTr("Backup failed")
                showMessage.open()
            }
        }

        Text {
            Layout.fillWidth: true
            text: qsTr("Recent backups")
            font.pixelSize: Theme.fontSizeCaption
            font.letterSpacing: 0.5
            color: Theme.mutedInk
            visible: backupRepeater.count > 0
        }

        Repeater {
            id: backupRepeater
            model: AppBackend.backups()

            delegate: Button {
                required property string modelData
                Layout.fillWidth: true
                flat: true
                text: modelData
                implicitHeight: Theme.touchTarget
                onClicked: {
                    restoreConfirmDialog.backupPath = modelData
                    restoreConfirmDialog.open()
                }
            }
        }

        SectionLabel {
            Layout.topMargin: Theme.spacingM
            text: qsTr("Categories")
        }

        Button {
            text: qsTr("Manage categories")
            icon.source: "../icons/tag.svg"
            implicitHeight: Theme.touchTarget
            onClicked: screen.manageCategoriesRequested()
        }

        SectionLabel {
            Layout.topMargin: Theme.spacingM
            text: qsTr("About")
        }

        Text {
            text: qsTr("Masareef %1 — a simple expense tracker")
                .arg(AppBackend.version)
            font.pixelSize: Theme.fontSizeBody
            color: Theme.secondaryInk
        }
    }
}
