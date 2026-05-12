cmake_minimum_required(VERSION 3.21)
project(ChatExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml Widgets)

# 查找 qt-ai-desktop
find_package(qt-ai-desktop QUIET)
if(NOT qt-ai-desktop_FOUND)
    # 如果找不到，尝试从父目录构建
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}/qt-ai-desktop)
endif()

# 资源文件
set(RESOURCES
    resources/icons.qrc
    resources/themes.qrc
)

# QML 文件
set(QML_FILES
    qml/main.qml
    qml/ChatView.qml
    qml/MessageBubble.qml
    qml/SettingsPanel.qml
    qml/InputArea.qml
)

# C++ 源文件
set(SOURCES
    src/main.cpp
    src/ChatWindow.cpp
    src/MessageModel.cpp
)

set(HEADERS
    src/ChatWindow.h
    src/MessageModel.h
)

# 创建可执行文件
qt_add_executable(chat_app
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

qt_add_qml_module(chat_app
    URI "ChatExample"
    VERSION 1.0
    QML_FILES ${QML_FILES}
)

target_link_libraries(chat_app PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::Widgets
    qt-ai-desktop-core
)

# 设置输出目录
set_target_properties(chat_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
