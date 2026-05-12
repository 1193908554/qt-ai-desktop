import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: chatView
    color: "#0f3460"

    ListView {
        id: messageList
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12
        clip: true
        model: messageModel

        // 自动滚动到底部
        onCountChanged: {
            Qt.callLater(function() {
                messageList.positionViewAtEnd()
            })
        }

        delegate: MessageBubble {
            width: messageList.width
            messageContent: model.content
            messageSender: model.sender
            messageTimestamp: model.timestamp
            isStreaming: model.isStreaming
        }

        // 空状态提示
        Label {
            anchors.centerIn: parent
            visible: messageList.count === 0
            text: "Start a conversation...\nType a message below to begin"
            color: "#533483"
            font.pixelSize: 16
            horizontalAlignment: Text.AlignHCenter
        }
    }

    // 滚动到底部按钮
    Button {
        anchors.right: parent.right
        anchors.rightMargin: 24
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16
        visible: !atBottom
        text: "↓"
        width: 40
        height: 40
        onClicked: messageList.positionViewAtEnd()

        property bool atBottom: messageList.atYEnd
    }
}
