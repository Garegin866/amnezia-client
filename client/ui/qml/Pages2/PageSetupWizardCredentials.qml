import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import PageEnum 1.0

import "./"
import "../Controls2"
import "../Config"
import "../Controls2/TextTypes"

PageType {
    id: root

    BackButtonType {
        id: backButton

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 20
    }

    FlickableType {
        id: fl
        anchors.top: backButton.bottom
        anchors.bottom: parent.bottom
        contentHeight: content.height

        ColumnLayout {
            id: content

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.leftMargin: 16

            spacing: 16

            HeaderType {
                Layout.fillWidth: true

                headerText: qsTr("Server connection")
            }

            TextFieldWithHeaderType {
                id: hostname

                Layout.fillWidth: true
                headerText: qsTr("Server IP address [:port]")
                textFieldPlaceholderText: qsTr("255.255.255.255:88")
                textField.validator: RegularExpressionValidator {
                    regularExpression: InstallController.ipAddressPortRegExp()
                }
            }

            TextFieldWithHeaderType {
                id: username

                Layout.fillWidth: true
                headerText: qsTr("Login to connect via SSH")
                textFieldPlaceholderText: "root"
            }

            TextFieldWithHeaderType {
                id: secretData

                Layout.fillWidth: true
                headerText: qsTr("Password / SSH private key")
                textField.echoMode: TextInput.Password
            }

            BasicButtonType {
                Layout.fillWidth: true
                Layout.topMargin: 24

                text: qsTr("Set up a server the easy way")

                onClicked: function() {
                    if (!isCredentialsFilled()) {
                        return
                    }

                    InstallController.setShouldCreateServer(true)
                    InstallController.setCurrentlyInstalledServerCredentials(hostname.textField.text, username.textField.text, secretData.textField.text)

                    goToPage(PageEnum.PageSetupWizardEasy)
                }
            }

            BasicButtonType {
                Layout.fillWidth: true
                Layout.topMargin: -8

                defaultColor: "transparent"
                hoveredColor: Qt.rgba(1, 1, 1, 0.08)
                pressedColor: Qt.rgba(1, 1, 1, 0.12)
                disabledColor: "#878B91"
                textColor: "#D7D8DB"
                borderWidth: 1

                text: qsTr("Select protocol to install")

                onClicked: function() {
                    if (!isCredentialsFilled()) {
                        return
                    }

                    InstallController.setShouldCreateServer(true)
                    InstallController.setCurrentlyInstalledServerCredentials(hostname.textField.text, username.textField.text, secretData.textField.text)

                    goToPage(PageEnum.PageSetupWizardProtocols)
                }
            }
        }
    }

    function isCredentialsFilled() {
        var hasEmptyField = false

        if (hostname.textFieldText === "") {
            hostname.errorText = qsTr("Ip address cannot be empty")
            hasEmptyField = true
        } else if (!hostname.textField.acceptableInput) {
            hostname.errorText = qsTr("Enter the address in the format 255.255.255.255:88")
        }

        if (username.textFieldText === "") {
            username.errorText = qsTr("Login cannot be empty")
            hasEmptyField = true
        }
        if (secretData.textFieldText === "") {
            secretData.errorText = qsTr("Password/private key cannot be empty")
            hasEmptyField = true
        }
        return !hasEmptyField
    }
}
