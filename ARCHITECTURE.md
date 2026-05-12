# Qt AI Desktop 架构文档

## 概述

Qt AI Desktop 是一个用于构建 AI 驱动桌面应用程序的现代框架。它提供了统一的 API 接口，支持多种 AI 后端，并提供精美的 QML UI 组件。

## 架构图

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

## 核心模块

### 1. AIEngine（AI 引擎）

负责 AI 推理的核心接口，提供统一的 API 用于模型加载、推理和流式响应。

**主要功能：**
- 模型加载和卸载
- 同步和异步推理
- 流式 token 生成
- 参数配置

**信号：**
- `tokenGenerated(QString token)` - 流式 token 生成
- `responseComplete(QString response)` - 响应完成
- `errorOccurred(QString error)` - 错误发生

### 2. ModelManager（模型管理器）

负责模型的管理和元数据存储。

**主要功能：**
- 添加和移除模型
- 获取模型列表
- 模型元数据查询

### 3. 后端插件

支持多种 AI 后端：

- **llama.cpp** - LLM 推理引擎
- **ONNX Runtime** - 通用 AI 推理
- **whisper.cpp** - 语音识别

## 设计决策

### 1. C++20 Concepts

使用 C++20 Concepts 约束后端接口，提供类型安全的插件系统。

### 2. Pimpl 模式

使用 Pimpl（Pointer to Implementation）模式隔离第三方头文件，防止编译污染。

### 3. 信号驱动

使用 Qt 信号槽机制实现异步通信，天然支持流式响应。

### 4. 跨平台

支持 Windows、Linux、macOS，使用 Qt 的跨平台能力。

## 目录结构

```
qt-ai-desktop/
├── include/
│   └── qt-ai-desktop/
│       ├── ai_engine.h
│       └── model_manager.h
├── src/
│   ├── ai_engine.cpp
│   ├── model_manager.cpp
│   ├── backends/
│   │   ├── llama/
│   │   ├── onnx/
│   │   └── whisper/
│   └── qml/
├── examples/
│   └── simple_chat/
├── docs/
│   ├── API.md
│   └── QUICKSTART.md
├── CMakeLists.txt
├── README.md
└── LICENSE
```

## 技术栈

| 技术 | 版本 | 用途 |
|------|------|------|
| Qt | 6.7+ | 跨平台 GUI 框架 |
| C++ | 20 | 核心语言 |
| CMake | 3.20+ | 构建系统 |
| llama.cpp | latest | LLM 推理 |
| ONNX Runtime | latest | 通用 AI 推理 |
| whisper.cpp | latest | 语音识别 |

## 性能考虑

- 使用 SIMD 优化矩阵运算
- 智能模型加载和卸载
- 实时流式响应
- 内存池管理

## 扩展性

- 插件化后端架构
- 可扩展 UI 组件
- 自定义参数配置
- 事件驱动设计