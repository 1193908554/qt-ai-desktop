cmake_minimum_required(VERSION 3.20)

project(qt-ai-desktop
    VERSION 1.0.0
    DESCRIPTION "Qt + AI Desktop Application Framework"
    HOMEPAGE_URL "https://github.com/1193908554/qt-ai-desktop"
    LANGUAGES CXX
)

# =============================================================================
# C++ 标准
# =============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================================================
# Qt6 配置
# =============================================================================
find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Quick
    Qml
)

# =============================================================================
# 选项配置
# =============================================================================
option(QTAI_BUILD_EXAMPLES "Build example applications" ON)
option(QTAI_BUILD_TESTS "Build tests" OFF)
option(QTAI_USE_LLAMA_CPP "Enable llama.cpp backend" ON)
option(QTAI_USE_ONNX_RUNTIME "Enable ONNX Runtime backend" OFF)
option(QTAI_USE_VULKAN "Enable Vulkan compute backend" OFF)

# =============================================================================
# 编译定义
# =============================================================================
add_compile_definitions(
    QT_DEPRECATED_WARNINGS
    QT_NO_KEYWORDS
    $<$<CONFIG:Debug>:QTAI_DEBUG>
)

# =============================================================================
# 核心库 - qt-ai-desktop-core
# =============================================================================
set(QTAI_CORE_SOURCES
    src/ai_engine.cpp
    src/model_manager.cpp
    src/ai_chat_model.cpp
    src/backend.cpp
    src/simulation_backend.cpp
    src/logger.cpp
    src/config.cpp
)

set(QTAI_CORE_HEADERS
    include/qt-ai-desktop/ai_engine.h
    include/qt-ai-desktop/model_manager.h
    include/qt-ai-desktop/ai_chat_model.h
    include/qt-ai-desktop/backend.h
    include/qt-ai-desktop/simulation_backend.h
    include/qt-ai-desktop/logger.h
    include/qt-ai-desktop/config.h
)

add_library(qt-ai-desktop-core STATIC
    ${QTAI_CORE_SOURCES}
    ${QTAI_CORE_HEADERS}
)

target_include_directories(qt-ai-desktop-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(qt-ai-desktop-core
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::Qml
)

set_target_properties(qt-ai-desktop-core PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    AUTOMOC ON
)

# =============================================================================
# QML 模块注册
# =============================================================================
qt_add_qml_module(qt-ai-desktop-core
    URI "QtAIDesktop"
    VERSION "1.0"
    QML_FILES
    SOURCES
        ${QTAI_CORE_HEADERS}
        ${QTAI_CORE_SOURCES}
)

# =============================================================================
# 后端集成（可选）
# =============================================================================
if(QTAI_USE_LLAMA_CPP)
    # TODO: 集成 llama.cpp
    # find_package(llama REQUIRED)
    # target_link_libraries(qt-ai-desktop-core PRIVATE llama)
    message(STATUS "llama.cpp backend: ENABLED (needs integration)")
endif()

if(QTAI_USE_ONNX_RUNTIME)
    # TODO: 集成 ONNX Runtime
    message(STATUS "ONNX Runtime backend: ENABLED (needs integration)")
endif()

if(QTAI_USE_VULKAN)
    # TODO: 集成 Vulkan
    message(STATUS "Vulkan compute backend: ENABLED (needs integration)")
endif()

# =============================================================================
# 示例应用
# =============================================================================
if(QTAI_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# =============================================================================
# 测试
# =============================================================================
if(QTAI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# =============================================================================
# 安装配置
# =============================================================================
include(GNUInstallDirs)

install(TARGETS qt-ai-desktop-core
    EXPORT qt-ai-desktop-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/qt-ai-desktop
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT qt-ai-desktop-targets
    FILE qt-ai-desktop-targets.cmake
    NAMESPACE QtAIDesktop::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qt-ai-desktop
)

# =============================================================================
# CMake 包配置
# =============================================================================
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/qt-ai-desktop-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/qt-ai-desktop-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qt-ai-desktop
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/qt-ai-desktop-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/qt-ai-desktop-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/qt-ai-desktop-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qt-ai-desktop
)

# =============================================================================
# 打包配置
# =============================================================================
set(CPACK_PACKAGE_NAME "qt-ai-desktop")
set(CPACK_PACKAGE_VENDOR "LinXi")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "https://github.com/1193908554/qt-ai-desktop")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
set(CPACK_STRIP_FILES TRUE)

include(CPack)
