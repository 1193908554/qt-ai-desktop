import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: inputArea
    color: "#16213e"

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // 输入框
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 12
            color: "#0f3460"
            border.color: "#533483"
            border.width: 1

            ScrollView {
                anchors.fill: parent
                anchors.margins: 8

                TextArea {
                    id: inputField
                    placeholderText: "Type your message..."
                    color: "#ffffff"
                    placeholderTextColor: "#533483"
                    font.pixelSize: 14
                    wrapMode: Text.Wrap
                    selectByMouse: true

                    background: Item {}

                    Keys.onPressed: function(event) {
                        if (event.key === Qt.Key_Return && !(event.modifiers & Qt.ShiftModifier)) {
                            event.accepted = true
                            sendMessage()
                        }
                    }
                }
            }
        }

        // 发送按钮
        Button {
            Layout.preferredWidth: 60
            Layout.preferredHeight: 60
            text: "➤"
            font.pixelSize: 24

            background: Rectangle {
                radius: 30
                color: parent.pressed ? "#c73e54" : (parent.hovered ? "#d63447" : "#e94560")
            }

            contentItem: Label {
                text: parent.text
                font: parent.font
                color: "#ffffff"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: sendMessage()
        }
    }

    function sendMessage() {
        var text = inputField.text.trim()
        if (text.length > 0) {
            chatWindow.sendMessage(text)
            inputField.clear()
        }
    }
}
