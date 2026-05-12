import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: messageBubble
    height: bubbleLayout.height + 20

    property string messageContent: ""
    property int messageSender: 0  // 0 = User, 1 = Assistant, 2 = System
    property var messageTimestamp: new Date()
    property bool isStreaming: false

    RowLayout {
        id: bubbleLayout
        anchors.left: messageSender === 0 ? undefined : parent.left
        anchors.right: messageSender === 0 ? parent.right : undefined
        anchors.leftMargin: messageSender === 0 ? 60 : 16
        anchors.rightMargin: messageSender === 0 ? 16 : 60
        spacing: 8

        // 头像
        Rectangle {
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignTop
            radius: 18
            color: messageSender === 0 ? "#e94560" : "#533483"
            visible: messageSender !== 2

            Label {
                anchors.centerIn: parent
                text: messageSender === 0 ? "👤" : "🤖"
                font.pixelSize: 18
            }
        }

        // 消息气泡
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(contentLabel.implicitHeight + 24, 36)
            radius: 12
            color: {
                if (messageSender === 0) return "#e94560"
                if (messageSender === 1) return "#533483"
                return "#16213e"
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12

                Label {
                    id: contentLabel
                    Layout.fillWidth: true
                    text: messageContent || (isStreaming ? "Thinking..." : "")
                    color: "#ffffff"
                    wrapMode: Text.Wrap
                    font.pixelSize: 14
                    textFormat: Text.MarkdownText
                }

                Label {
                    Layout.fillWidth: true
                    text: Qt.formatTime(messageTimestamp, "hh:mm")
                    color: "#ffffff80"
                    font.pixelSize: 10
                    horizontalAlignment: messageSender === 0 ? Text.AlignRight : Text.AlignLeft
                }
            }

            // 流式输入光标
            Rectangle {
                visible: isStreaming
                width: 2
                height: 16
                color: "#ffffff"
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 12
                anchors.right: messageSender === 0 ? parent.right : undefined
                anchors.left: messageSender === 0 ? undefined : parent.left
                anchors.leftMargin: messageSender === 0 ? undefined : 12
                anchors.rightMargin: messageSender === 0 ? 12 : undefined

                SequentialAnimation on opacity {
                    loops: Animation.Infinite
                    running: isStreaming
                    NumberAnimation { to: 1; duration: 500 }
                    NumberAnimation { to: 0; duration: 500 }
                }
            }
        }
    }
}
