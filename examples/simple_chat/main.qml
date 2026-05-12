import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtAIDesktop

/**
 * Simple Chat Demo - AI Chat 应用示例
 */
ApplicationWindow {
    id: root
    
    visible: true
    width: 800
    height: 600
    title: "AI Chat Demo - Qt AI Desktop"
    
    // 颜色定义
    readonly property color primaryColor: "#2196F3"
    readonly property color secondaryColor: "#1976D2"
    readonly property color userBubbleColor: "#E3F2FD"
    readonly property color aiBubbleColor: "#F5F5F5"
    readonly property color backgroundColor: "#FAFAFA"
    
    // 当前选中的模型
    property string selectedModelId: ""
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        // ============================================================
        // 工具栏
        // ============================================================
        ToolBar {
            Layout.fillWidth: true
            
            background: Rectangle {
                color: primaryColor
            }
            
            RowLayout {
                anchors.fill: parent
                
                ToolButton {
                    text: "☰"
                    font.pixelSize: 20
                    onClicked: modelDrawer.open()
                }
                
                Label {
                    text: "AI Chat Demo"
                    font.pixelSize: 18
                    font.bold: true
                    color: "white"
                    Layout.fillWidth: true
                }
                
                // 模型状态指示器
                Rectangle {
                    width: 12
                    height: 12
                    radius: 6
                    color: aiEngine.isLoaded ? "#4CAF50" : "#F44336"
                    
                    ToolTip.visible: hovered
                    ToolTip.text: aiEngine.isLoaded ? 
                        "Model: " + aiEngine.modelName : 
                        "No model loaded"
                    
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                    }
                }
                
                ToolButton {
                    text: "⚙"
                    font.pixelSize: 20
                    onClicked: settingsDialog.open()
                }
            }
        }
        
        // ============================================================
        // 主内容区
        // ============================================================
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            
            // ============================================================
            // 聊天消息列表
            // ============================================================
            ListView {
                id: chatListView
                
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                spacing: 12
                
                model: chatModel
                
                // 当有新消息时自动滚动到底部
                onCountChanged: {
                    Qt.callLater(function() {
                        chatListView.positionViewAtEnd();
                    })
                }
                
                // 空状态提示
                Label {
                    anchors.centerIn: parent
                    text: "开始一段对话吧！\n从左侧选择一个模型开始"
                    font.pixelSize: 16
                    color: "#9E9E9E"
                    visible: chatListView.count === 0
                    horizontalAlignment: Text.AlignHCenter
                }
                
                delegate: Item {
                    width: chatListView.width
                    height: bubble.height + 20
                    
                    // 用户消息靠右，AI消息靠左
                    readonly property bool isUser: model.role === 1  // MessageRole.User = 1
                    
                    Rectangle {
                        id: bubble
                        
                        anchors {
                            left: isUser ? undefined : parent.left
                            right: isUser ? parent.right : undefined
                            margins: 16
                        }
                        
                        width: Math.min(parent.width * 0.7, contentLabel.implicitWidth + 32)
                        height: contentLabel.implicitHeight + 24
                        
                        radius: 12
                        color: isUser ? userBubbleColor : aiBubbleColor
                        
                        // 阴影效果
                        layer.enabled: true
                        layer.effect: DropShadow {
                            radius: 4
                            samples: 8
                            horizontalOffset: 0
                            verticalOffset: 2
                            color: "#20000000"
                        }
                        
                        Label {
                            id: roleLabel
                            
                            anchors {
                                top: parent.top
                                left: parent.left
                                margins: 8
                            }
                            
                            text: isUser ? "You" : "AI"
                            font.pixelSize: 11
                            font.bold: true
                            color: isUser ? primaryColor : "#4CAF50"
                        }
                        
                        TextEdit {
                            id: contentLabel
                            
                            anchors {
                                top: roleLabel.bottom
                                left: parent.left
                                right: parent.right
                                margins: 8
                            }
                            
                            text: model.content
                            wrapMode: TextEdit.Wrap
                            readOnly: true
                            selectByMouse: true
                            
                            // 流式输出时显示光标
                            BusyIndicator {
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                running: model.isStreaming
                                width: 16
                                height: 16
                            }
                        }
                    }
                }
            }
        }
        
        // ============================================================
        // 输入区域
        // ============================================================
        Rectangle {
            Layout.fillWidth: true
            height: 70
            color: "white"
            
            // 顶部分割线
            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: "#E0E0E0"
            }
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8
                
                // 输入框
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 20
                    color: "#F5F5F5"
                    border.color: inputField.activeFocus ? primaryColor : "#E0E0E0"
                    border.width: 1
                    
                    TextField {
                        id: inputField
                        
                        anchors.fill: parent
                        anchors.margins: 2
                        
                        placeholderText: "输入消息... (Enter 发送, Shift+Enter 换行)"
                        font.pixelSize: 14
                        
                        background: Rectangle {
                            radius: 20
                            color: "transparent"
                        }
                        
                        // 发送消息
                        onAccepted: {
                            if (text.trim().length > 0) {
                                chatModel.sendMessage(text.trim());
                                text = "";
                            }
                        }
                    }
                }
                
                // 发送按钮
                Button {
                    id: sendButton
                    
                    Layout.preferredWidth: 60
                    Layout.fillHeight: true
                    
                    enabled: inputField.text.trim().length > 0 && 
                             aiEngine.isLoaded &&
                             !chatModel.isGenerating
                    
                    background: Rectangle {
                        radius: 20
                        color: sendButton.enabled ? primaryColor : "#BDBDBD"
                    }
                    
                    contentItem: Text {
                        text: chatModel.isGenerating ? "⏹" : "➤"
                        font.pixelSize: 20
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        if (chatModel.isGenerating) {
                            aiEngine.stop();
                        } else {
                            inputField.accepted();
                        }
                    }
                }
            }
        }
    }
    
    // ============================================================
    // 模型选择抽屉
    // ============================================================
    Drawer {
        id: modelDrawer
        
        width: 280
        height: root.height
        
        background: Rectangle {
            color: "white"
        }
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 0
            
            // 标题
            Rectangle {
                Layout.fillWidth: true
                height: 56
                color: primaryColor
                
                Label {
                    anchors.centerIn: parent
                    text: "模型列表"
                    font.pixelSize: 18
                    font.bold: true
                    color: "white"
                }
            }
            
            // 已加载模型信息
            Rectangle {
                Layout.fillWidth: true
                height: 80
                color: "#E3F2FD"
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    
                    Label {
                        text: aiEngine.isLoaded ? 
                            "✓ " + aiEngine.modelName : 
                            "未加载模型"
                        font.pixelSize: 14
                        font.bold: true
                        color: aiEngine.isLoaded ? "#4CAF50" : "#F44336"
                    }
                    
                    Label {
                        text: aiEngine.isLoaded ?
                            "状态: 就绪 | Token: " + aiEngine.tokenCount :
                            "请从下方选择模型"
                        font.pixelSize: 12
                        color: "#666"
                    }
                    
                    // 加载进度条
                    ProgressBar {
                        Layout.fillWidth: true
                        visible: aiEngine.status === 1  // Loading
                        value: aiEngine.progress
                    }
                }
            }
            
            // 分割线
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#E0E0E0"
            }
            
            // 模型列表
            ListView {
                id: modelListView
                
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                
                model: ListModel {
                    // 示例模型列表
                    ListElement {
                        name: "Llama-3.2-3B-Q4"
                        family: "Llama"
                        size: "2.1 GB"
                        status: "ready"
                    }
                    ListElement {
                        name: "Mistral-7B-Q5"
                        family: "Mistral"
                        size: "5.3 GB"
                        status: "download"
                    }
                    ListElement {
                        name: "Phi-3-Mini-Q4"
                        family: "Phi"
                        size: "2.4 GB"
                        status: "download"
                    }
                }
                
                delegate: ItemDelegate {
                    width: modelListView.width
                    height: 70
                    
                    background: Rectangle {
                        color: selectedModelId === name ? "#E3F2FD" : 
                               hovered ? "#F5F5F5" : "transparent"
                    }
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12
                        
                        // 图标
                        Rectangle {
                            width: 40
                            height: 40
                            radius: 8
                            color: "#2196F3"
                            
                            Label {
                                anchors.centerIn: parent
                                text: "🤖"
                                font.pixelSize: 20
                            }
                        }
                        
                        // 信息
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            
                            Label {
                                text: name
                                font.pixelSize: 14
                                font.bold: true
                                elide: Text.ElideRight
                            }
                            
                            Label {
                                text: family + " | " + size
                                font.pixelSize: 12
                                color: "#666"
                            }
                        }
                        
                        // 操作按钮
                        Button {
                            text: status === "ready" ? "✓" : "⬇"
                            enabled: status === "download"
                            
                            onClicked: {
                                selectedModelId = name;
                                // TODO: 实际加载/下载模型
                                console.log("Selected model:", name);
                            }
                        }
                    }
                }
            }
            
            // 底部按钮
            Rectangle {
                Layout.fillWidth: true
                height: 60
                color: "#FAFAFA"
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    
                    Button {
                        Layout.fillWidth: true
                        text: "扫描本地模型"
                        onClicked: {
                            modelManager.scanLocalModels();
                        }
                    }
                }
            }
        }
    }
    
    // ============================================================
    // 设置对话框
    // ============================================================
    Dialog {
        id: settingsDialog
        
        title: "设置"
        modal: true
        anchors.centerIn: parent
        width: 400
        height: 500
        
        // TODO: 添加设置内容
        
        standardButtons: Dialog.Ok | Dialog.Cancel
    }
}
