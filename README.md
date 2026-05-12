<div align="center">

# Qt AI Desktop

🚀 **A Modern Framework for Building AI-Powered Desktop Applications**

[![Qt](https://img.shields.io/badge/Qt-6.7+-41CD52?style=for-the-badge&logo=qt&logoColor=white)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C++-20-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-blue?style=for-the-badge)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey?style=for-the-badge)]()

[English](README.md) | [中文](README_CN.md) | [架构文档](ARCHITECTURE.md) | [API 文档](docs/API.md) | [快速开始](docs/QUICKSTART.md)

</div>

---

## ✨ 特性

- 🤖 **多后端支持** - 集成 llama.cpp、ONNX Runtime、whisper.cpp
- 🎨 **现代化 UI** - 基于 Qt6/QML 的精美界面组件
- ⚡ **高性能** - C++20 协程 + SIMD 优化
- 🔌 **插件化架构** - 轻松扩展新的 AI 后端
- 📱 **跨平台** - 支持 Windows、Linux、macOS
- 📦 **零依赖** - 核心层仅依赖 Qt
- 📝 **结构化日志** - 完整的日志系统
- ⚙️ **配置管理** - JSON 配置文件支持
- 🧪 **完整测试** - GoogleTest 单元测试
- 🔄 **CI/CD** - GitHub Actions 多平台构建
- 🦙 **llama.cpp** - GGUF 模型推理支持

## 🚀 快速开始

### 安装

```bash
git clone https://github.com/1193908554/qt-ai-desktop.git
cd qt-ai-desktop
cmake -B build
cmake --build build
```

### 最小示例

```cpp
#include <QApplication>
#include <qt-ai-desktop/ai_engine.h>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 创建 AI 引擎
    QtAIDesktop::AIEngine engine;
    
    // 加载模型
    engine.loadModel("path/to/model.gguf");
    
    // 开始对话
    auto response = engine.chat("Hello, AI!");
    qDebug() << response;
    
    return app.exec();
}
```

## 📚 文档

- [快速开始指南](docs/QUICKSTART.md) - 15 分钟上手
- [架构文档](ARCHITECTURE.md) - 深入了解设计
- [API 文档](docs/API.md) - 完整 API 参考
- [贡献指南](CONTRIBUTING.md) - 参与开发

## 🏗️ 架构

```
┌─────────────────────────────────────────────┐
│              应用层 (Application)            │
│   Chat App │ Image App │ Voice App │ ...    │
├─────────────────────────────────────────────┤
│              UI 组件层 (Components)          │
│   AIChatView │ ModelSelector │ StreamText   │
├─────────────────────────────────────────────┤
│              核心框架层 (Core)               │
│   AIEngine │ ModelManager │ StreamManager   │
├─────────────────────────────────────────────┤
│              后端插件层 (Backends)           │
│   llama.cpp │ ONNX Runtime │ whisper.cpp    │
└─────────────────────────────────────────────┘
```

## 🛠️ 技术栈

| 技术 | 用途 |
|------|------|
| **Qt 6.7+** | 跨平台 GUI 框架 |
| **C++20** | 现代 C++ 特性 |
| **llama.cpp** | LLM 推理引擎 |
| **ONNX Runtime** | 通用 AI 推理 |
| **whisper.cpp** | 语音识别 |
| **CMake** | 构建系统 |

## 📊 性能

- ⚡ **推理速度** - CPU 上 50+ tokens/s（llama.cpp）
- 🎯 **内存优化** - 智能模型加载和卸载
- 📈 **实时流式** - 逐字输出，低延迟响应

## 🤝 贡献

欢迎贡献！请查看 [贡献指南](CONTRIBUTING.md) 了解详情。

## 📄 许可证

本项目采用 [MIT 许可证](LICENSE) - 详见 LICENSE 文件

## 👨‍💻 作者

**LinXi** - [GitHub](https://github.com/1193908554) | [CSDN](https://blog.csdn.net/qq_45254369)

---

<div align="center">

**如果这个项目对你有帮助，请给个 ⭐ Star 支持一下！**

</div>