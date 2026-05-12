# Changelog

本文件记录 Qt AI Desktop 项目的所有重要变更。

格式基于 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/)，
版本号遵循 [语义化版本](https://semver.org/lang/zh-CN/)。

## [Unreleased]

### 新增
- GitHub Actions CI/CD 多平台构建
- 插件化后端架构（IBackend 接口）
- 模拟后端（用于测试和演示）
- 结构化日志系统（Logger）
- JSON 配置管理系统（Config）
- GoogleTest 单元测试框架
- CONTRIBUTING.md 贡献指南
- CHANGELOG.md 变更日志

### 改进
- AIEngine: 完善异步推理和流式输出
- ModelManager: 增强模型扫描和元数据解析
- AIChatModel: 优化 QML 绑定和消息管理
- CMakeLists.txt: 完善构建配置和安装规则

## [1.0.0] - 2026-05-12

### 新增

#### 核心组件
- **AIEngine** - 统一 AI 推理引擎
  - 同步/异步推理支持
  - 流式 token 输出
  - 多后端支持（llama.cpp, ONNX Runtime, whisper.cpp, Vulkan）
  - 可配置生成参数（temperature, topP, topK 等）
  - 模型加载/卸载管理
  - 生成进度和统计

- **ModelManager** - 模型管理器
  - 本地模型目录扫描
  - 模型元数据自动解析
  - 模型收藏系统
  - 搜索和过滤
  - 导入/导出功能

- **AIChatModel** - QML 聊天数据模型
  - 实时流式更新
  - 上下文管理
  - 消息历史
  - 类型安全的 QML 绑定

#### 示例应用
- **Simple Chat** - 完整聊天界面
  - 基于 Qt Quick 的 UI
  - 模型选择面板
  - 流式消息显示
  - 响应式设计

#### 文档
- README.md - 项目说明和快速开始
- ARCHITECTURE.md - 架构设计文档
- API 文档 - 完整 API 参考
- QUICKSTART.md - 快速入门指南
- 示例代码和注释

#### 构建系统
- CMake 构建配置
- 跨平台支持（Windows, Linux, macOS）
- 可选组件配置
- 安装规则

### 技术栈
- C++20
- Qt 6.7+
- CMake 3.20+
- llama.cpp（后端）
- ONNX Runtime（后端）
- whisper.cpp（后端）
- Vulkan Compute（后端）

---

## 版本说明

### 版本号规则

- **主版本号 (Major)**: 不兼容的 API 变更
- **次版本号 (Minor)**: 向后兼容的功能新增
- **修订号 (Patch)**: 向后兼容的问题修复

### 发布周期

- 主版本：每年 1-2 次
- 次版本：每季度
- 修订号：按需发布

### 支持政策

- 主版本：支持 2 年
- 次版本：支持 1 年
- 安全更新：所有活跃版本
