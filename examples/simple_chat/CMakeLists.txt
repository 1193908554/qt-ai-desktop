# =============================================================================
# Simple Chat Example
# =============================================================================

set(EXAMPLE_SOURCES
    main.cpp
)

set(EXAMPLE_QML
    main.qml
)

# 添加可执行文件
qt_add_executable(simple_chat
    ${EXAMPLE_SOURCES}
)

# 设置属性
set_target_properties(simple_chat PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    AUTOMOC ON
    AUTORCC ON
)

# 链接库
target_link_libraries(simple_chat
    PRIVATE
        QtAIDesktop::qt-ai-desktop-core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Quick
)

# 添加 QML 模块
qt_add_qml_module(simple_chat
    URI "SimpleChat"
    VERSION "1.0"
    QML_FILES ${EXAMPLE_QML}
)

# =============================================================================
# 安装
# =============================================================================
install(TARGETS simple_chat
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
