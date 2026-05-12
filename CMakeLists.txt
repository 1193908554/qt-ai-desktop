cmake_minimum_required(VERSION 3.20)

project(qt-ai-desktop
    VERSION 0.1.0
    DESCRIPTION "Qt + AI Desktop Application Framework"
    HOMEPAGE_URL "https://github.com/1193908554/qt-ai-desktop"
    LANGUAGES CXX
)

# 设置 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 查找 Qt6 包
find_package(Qt6 6.7 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
)

# 启用 Qt 自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 选项
option(QT_AI_DESKTOP_BUILD_EXAMPLES "Build examples" ON)
option(QT_AI_DESKTOP_BUILD_TESTS "Build tests" OFF)
option(QT_AI_DESKTOP_USE_LLAMA_CPP "Use llama.cpp backend" ON)
option(QT_AI_DESKTOP_USE_ONNX_RUNTIME "Use ONNX Runtime backend" OFF)
option(QT_AI_DESKTOP_USE_WHISPER_CPP "Use whisper.cpp backend" OFF)

# 添加子目录
add_subdirectory(src)

# 构建示例
if(QT_AI_DESKTOP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# 安装配置
include(GNUInstallDirs)

# 导出目标
install(EXPORT qt-ai-desktop-targets
    FILE QtAITargets.cmake
    NAMESPACE QtAI::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtAI
)

# 生成配置文件
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/QtAIConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/QtAIConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/QtAIConfig.cmake"
    @ONLY
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/QtAIConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/QtAIConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtAI
)

# 打印配置信息
message(STATUS "Qt AI Desktop v${PROJECT_VERSION}")
message(STATUS "  Qt Version: ${Qt6_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Examples: ${QT_AI_DESKTOP_BUILD_EXAMPLES}")
message(STATUS "  llama.cpp: ${QT_AI_DESKTOP_USE_LLAMA_CPP}")
message(STATUS "  ONNX Runtime: ${QT_AI_DESKTOP_USE_ONNX_RUNTIME}")
message(STATUS "  whisper.cpp: ${QT_AI_DESKTOP_USE_WHISPER_CPP}")