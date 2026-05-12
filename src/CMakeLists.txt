cmake_minimum_required(VERSION 3.20)

# 查找 Qt6
find_package(Qt6 6.7 REQUIRED COMPONENTS Core Gui Qml Quick)

# 启用自动处理
set(CMAKE_AUTOMOC ON)

# 创建核心库
add_library(QtAI_Core
    ai_engine.cpp
    model_manager.cpp
)

# 设置库属性
target_include_directories(QtAI_Core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接 Qt
target_link_libraries(QtAI_Core
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
)

# 设置输出名称
set_target_properties(QtAI_Core PROPERTIES
    OUTPUT_NAME QtAI_Core
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# 安装
install(TARGETS QtAI_Core
    EXPORT QtAITargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 安装头文件
install(FILES
    ${CMAKE_SOURCE_DIR}/include/qt-ai-desktop/ai_engine.h
    ${CMAKE_SOURCE_DIR}/include/qt-ai-desktop/model_manager.h
    DESTINATION include/qt-ai-desktop
)

# 添加后端
if(QT_AI_DESKTOP_USE_LLAMA_CPP)
    add_subdirectory(backends/llama)
endif()

if(QT_AI_DESKTOP_USE_ONNX_RUNTIME)
    add_subdirectory(backends/onnx)
endif()

if(QT_AI_DESKTOP_USE_WHISPER_CPP)
    add_subdirectory(backends/whisper)
endif()