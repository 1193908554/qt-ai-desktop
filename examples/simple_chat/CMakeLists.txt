cmake_minimum_required(VERSION 3.20)

project(simple_chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt6
find_package(Qt6 6.7 REQUIRED COMPONENTS Core Gui Widgets)

# 启用自动处理
set(CMAKE_AUTOMOC ON)

# 创建可执行文件
add_executable(simple_chat main.cpp)

# 链接库
target_link_libraries(simple_chat
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        QtAI::Core
)

# 包含目录
target_include_directories(simple_chat
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)