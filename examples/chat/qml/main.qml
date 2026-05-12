import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: root
    visible: true
    width: 800
    height: 600
    title: "Qt AI Desktop - Chat"
    color: "#1a1a2e"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 顶部工具栏
        Rectangle {
            Layout.fillWidth: true
            height: 50
            color: "#16213e"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16

                Label {
                    text: "🤖 Qt AI Desktop"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#e94560"
                }

                Item { Layout.fillWidth: true }

                ComboBox {
                    id: backendSelector
                    model: ["simulation", "llamacpp", "onnxruntime"]
                    currentIndex: 0
                    onCurrentTextChanged: chatWindow.switchBackend(currentText)
                }

                Button {
                    text: "Clear"
                    onClicked: chatWindow.clearHistory()
                }
            }
        }

        // 聊天视图
        ChatView {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        // 输入区域
        InputArea {
            Layout.fillWidth: true
            Layout.preferredHeight: 100
        }
    }
}
