# =============================================================================
# 单元测试
# =============================================================================

# 查找 GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

# Windows 下设置 Runtime
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 启用 CTest
enable_testing()

# =============================================================================
# 测试可执行文件
# =============================================================================
set(TEST_SOURCES
    test_ai_engine.cpp
    test_model_manager.cpp
    test_config.cpp
    test_simulation_backend.cpp
)

add_executable(qt-ai-desktop-tests ${TEST_SOURCES})

target_link_libraries(qt-ai-desktop-tests
    PRIVATE
        qt-ai-desktop-core
        GTest::gtest_main
        Qt6::Core
        Qt6::Gui
)

target_include_directories(qt-ai-desktop-tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(qt-ai-desktop-tests PROPERTIES
    AUTOMOC ON
)

# =============================================================================
# 注册测试
# =============================================================================
include(GoogleTest)
gtest_discover_tests(qt-ai-desktop-tests)
