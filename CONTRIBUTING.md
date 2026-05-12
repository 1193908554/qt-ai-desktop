# 贡献指南

感谢你对 Qt AI Desktop 项目的关注！我们欢迎各种形式的贡献。

## 🚀 如何贡献

### 报告 Bug

1. 在 [Issues](https://github.com/1193908554/qt-ai-desktop/issues) 页面搜索是否已有相同问题
2. 如果没有，创建新的 Issue，包含：
   - 清晰的标题和描述
   - 复现步骤
   - 期望行为和实际行为
   - 环境信息（操作系统、Qt 版本、编译器版本）
   - 相关日志或截图

### 提出新功能

1. 在 Issues 中创建 Feature Request
2. 描述功能的使用场景和价值
3. 如果可能，提供设计草案

### 提交代码

1. **Fork** 本仓库
2. 创建功能分支：`git checkout -b feature/amazing-feature`
3. 提交更改：`git commit -m 'feat: add amazing feature'`
4. 推送到分支：`git push origin feature/amazing-feature`
5. 创建 **Pull Request**

## 📝 开发规范

### Commit 规范

使用 [Conventional Commits](https://www.conventionalcommits.org/) 格式：

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

**类型 (type):**
- `feat`: 新功能
- `fix`: Bug 修复
- `docs`: 文档更新
- `style`: 代码格式（不影响逻辑）
- `refactor`: 重构
- `perf`: 性能优化
- `test`: 测试相关
- `chore`: 构建/工具相关

**示例：**
```
feat(ai-engine): add streaming inference support

- Implement token-by-token streaming via callback
- Add stop generation capability
- Update documentation with streaming examples

Closes #123
```

### 代码规范

#### C++ 代码风格

- 使用 C++20 特性
- 遵循 [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- 命名约定：
  - 类名：`PascalCase`（如 `AIEngine`）
  - 方法名：`camelCase`（如 `loadModel`）
  - 成员变量：`m_` 前缀（如 `m_modelLoaded`）
  - 常量：`UPPER_SNAKE_CASE`
  - 文件名：`snake_case`（如 `ai_engine.cpp`）

#### Qt 相关

- 使用 Qt 的信号槽机制
- 使用 `Q_OBJECT` 宏
- 使用 Qt 的容器和字符串类型
- 遵循 Qt 的内存管理约定

### 文件组织

```
qt-ai-desktop/
├── include/qt-ai-desktop/   # 公共头文件
├── src/                      # 实现文件
├── examples/                 # 示例代码
├── tests/                    # 单元测试
├── docs/                     # 文档
└── .github/                  # GitHub Actions
```

## 🧪 测试

### 运行测试

```bash
# 配置测试
cmake -B build -DQTAI_BUILD_TESTS=ON

# 构建测试
cmake --build build

# 运行测试
cd build && ctest --output-on-failure
```

### 编写测试

- 使用 GoogleTest 框架
- 测试文件命名：`test_<module>.cpp`
- 每个公共 API 都应该有对应的测试
- 包含正常路径和边界条件测试

```cpp
#include <gtest/gtest.h>
#include "qt-ai-desktop/ai_engine.h"

TEST(AIEngineTest, LoadModel) {
    AIEngine engine;
    EXPECT_FALSE(engine.isLoaded());
    
    bool result = engine.loadModel("test.gguf");
    EXPECT_TRUE(result);
    EXPECT_TRUE(engine.isLoaded());
}
```

## 📚 文档

### API 文档

- 所有公共类和方法都需要 Doxygen 风格的注释
- 包含使用示例

```cpp
/**
 * @brief 加载 AI 模型
 * 
 * 从指定路径加载模型文件。支持 GGUF、ONNX 格式。
 * 加载过程是异步的，完成后会发出 @c loaded 信号。
 * 
 * @param path 模型文件路径
 * @param options 加载选项，支持：
 *   - gpu_layers (int): GPU 层数
 *   - context_length (int): 上下文长度
 * @return 是否成功开始加载
 * 
 * @example
 * @code
 * engine->loadModel("model.gguf", {{"gpu_layers", 32}});
 * @endcode
 * 
 * @see unloadModel(), loaded()
 */
Q_INVOKABLE bool loadModel(const QString &path, const QVariantMap &options = {});
```

### 用户文档

- 快速开始指南
- 架构说明
- API 参考
- 示例说明

## 🎯 开发路线

查看 [ROADMAP.md](ROADMAP.md) 了解项目规划。

当前优先事项：
1. 集成 llama.cpp 后端
2. 完善 QML 组件库
3. 添加模型下载管理
4. 性能优化和基准测试

## 📞 联系方式

- GitHub Issues: [qt-ai-desktop Issues](https://github.com/1193908554/qt-ai-desktop/issues)
- Email: 通过 GitHub 联系

## 📄 许可证

贡献的代码将采用 MIT 许可证。

---

感谢你的贡献！🙏
