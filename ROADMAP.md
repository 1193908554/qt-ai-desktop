# 🗺️ Qt AI Desktop 路线图

本文档记录 Qt AI Desktop 项目的开发计划和进度。

## 📋 当前版本: v1.1.0

### ✅ 已完成 (v1.0.0 - v1.1.0)

#### 核心架构
- [x] AIEngine 统一推理引擎接口
- [x] ModelManager 模型生命周期管理
- [x] AIChatModel QML 聊天数据模型
- [x] IBackend 插件化后端接口
- [x] BackendFactory 后端注册工厂
- [x] SimulationBackend 测试后端

#### 基础设施
- [x] CMake 构建系统
- [x] GitHub Actions CI/CD (Windows/Linux/macOS)
- [x] GoogleTest 单元测试框架
- [x] 结构化日志系统 (Logger)
- [x] JSON 配置管理 (Config)

#### 文档
- [x] README 项目说明
- [x] ARCHITECTURE 架构文档
- [x] API 参考文档
- [x] QUICKSTART 快速开始
- [x] CONTRIBUTING 贡献指南
- [x] CHANGELOG 变更日志

---

## 🚀 v1.2.0 (计划中)

### 🎯 核心功能

- [ ] **llama.cpp 后端集成**
  - GGUF 模型加载
  - 流式推理输出
  - GPU 加速支持
  - 多种量化格式

- [ ] **ONNX Runtime 后端**
  - ONNX 模型加载
  - 跨平台推理
  - GPU 加速 (CUDA/DirectML)

- [ ] **模型下载管理**
  - HuggingFace 模型仓库
  - 下载进度追踪
  - 模型版本管理

### 🎨 QML 组件库

- [ ] ChatView 聊天视图组件
- [ ] ModelSelector 模型选择器
- [ ] ProgressBar 进度条组件
- [ ] SettingsView 设置面板
- [ ] StatusBar 状态栏

---

## 🔮 v1.3.0 (规划中)

### 🎯 高级功能

- [ ] **多模态支持**
  - 图像生成 (Stable Diffusion)
  - 语音识别 (whisper.cpp)
  - 文本转语音

- [ ] **函数调用**
  - Function Calling 支持
  - Tool Use 接口
  - 插件系统

- [ ] **对话管理**
  - 多轮对话记忆
  - 上下文窗口管理
  - 对话导出/导入

### ⚡ 性能优化

- [ ] **推理优化**
  - KV Cache 量化
  - Flash Attention
  - 批处理优化

- [ ] **内存优化**
  - 模型卸载策略
  - 内存映射优化
  - GC 策略

---

## 🌟 v2.0.0 (远景)

### 🎯 平台扩展

- [ ] **WebAssembly 支持**
  - 浏览器端推理
  - Web UI

- [ ] **移动端支持**
  - Android (Qt for Android)
  - iOS (Qt for iOS)

- [ ] **云原生支持**
  - Docker 容器化
  - Kubernetes 部署
  - API 网关

### 🤖 AI 能力扩展

- [ ] **RAG (检索增强生成)**
  - 向量数据库集成
  - 文档索引
  - 上下文检索

- [ ] **Agent 框架**
  - 自主代理
  - 多代理协作
  - 工具链编排

- [ ] **微调支持**
  - LoRA 微调
  - 数据集管理
  - 训练监控

---

## 📊 进度追踪

| 版本 | 预计发布时间 | 状态 |
|------|-------------|------|
| v1.0.0 | 2026-05-12 | ✅ 已发布 |
| v1.1.0 | 2026-05-12 | ✅ 已发布 |
| v1.2.0 | 2026-06 | 🔄 开发中 |
| v1.3.0 | 2026-07 | 📝 规划中 |
| v2.0.0 | 2026-12 | 💭 愿景中 |

---

## 🤝 如何参与

查看 [CONTRIBUTING.md](CONTRIBUTING.md) 了解如何参与项目开发。

### 当前优先事项

1. **llama.cpp 后端集成** - 让项目能真正运行推理
2. **QML 组件库** - 丰富 UI 组件
3. **单元测试完善** - 提高测试覆盖率

### 贡献领域

- 🐛 Bug 修复
- ✨ 新功能开发
- 📚 文档完善
- 🧪 测试用例
- 🎨 UI/UX 设计
- 🌐 国际化翻译

---

感谢所有贡献者的支持！🙏
