# Qt AI Desktop - Chat Example

一个完整的 AI 聊天应用示例，展示如何使用 qt-ai-desktop 框架构建桌面 AI 应用。

## 功能特性

- 💬 多轮对话支持
- 🔄 流式输出（逐 token 显示）
- 📝 Markdown 渲染
- 🎨 现代化 UI 设计
- ⚙️ 模型配置面板
- 📊 推理性能统计

## 快速开始

### 前置要求

- Qt 6.7+
- CMake 3.21+
- C++20 编译器

### 构建运行

```bash
# 进入示例目录
cd examples/chat

# 创建构建目录
mkdir build && cd build

# 配置
cmake ..

# 编译
cmake --build .

# 运行
./chat_app
```

### 使用 llama.cpp 后端

```bash
# 启用 llama.cpp 支持
cmake -DQTAI_USE_LLAMA_CPP=ON ..

# 运行时指定模型
./chat_app --model /path/to/model.gguf
```

## 架构说明

```
chat/
├── CMakeLists.txt
├── main.cpp                 # 应用入口
├── ChatWindow.h/cpp        # 主窗口
├── ChatView.qml            # 聊天视图
├── MessageModel.h/cpp      # 消息数据模型
├── SettingsPanel.qml       # 设置面板
└── resources/
    ├── icons/              # 图标资源
    └── themes/             # 主题资源
```

## 代码示例

### 初始化 AI 引擎

```cpp
#include <qt-ai-desktop/ai_engine.h>
#include <qt-ai-desktop/backend_factory.h>

// 创建后端
auto backend = BackendFactory::create("simulation");
// 或使用 llama.cpp
// auto backend = BackendFactory::create("llamacpp");

// 初始化引擎
AIEngine engine;
engine.setBackend(std::move(backend));
engine.setModel("model.gguf");
```

### 流式对话

```cpp
// 发送消息并获取流式响应
engine.generateStream(prompt, [](const QString& token) {
    // 逐 token 更新 UI
    appendToChat(token);
});
```

### QML 集成

```qml
import QtAIDesktop 1.0

ChatView {
    model: chatModel
    onSendMessage: function(text) {
        engine.generateStream(text, function(token) {
            chatModel.appendToken(token)
        })
    }
}
```

## 截图

*（待添加截图）*

## 许可证

MIT License
