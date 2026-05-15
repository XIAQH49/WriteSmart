cmake_minimum_required(VERSION 3.20)
project(WriteSmart
    VERSION 2.0.0
    DESCRIPTION "智能文学创作客户端"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Core)

# ============================================================
# 模块化子项目（按依赖顺序，自底向上）
# ============================================================
add_subdirectory(src/utils)
add_subdirectory(src/network)
add_subdirectory(src/core/document)
add_subdirectory(src/core/ai)
add_subdirectory(src/core/plugin)
add_subdirectory(src/app)
add_subdirectory(src/ui)

# ============================================================
# 可执行文件入口
# ============================================================
set(RESOURCES resources/resources.qrc)

add_executable(${PROJECT_NAME}
    src/main.cpp
    ${RESOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ws_ui
    ws_app
    ws_plugin
    ws_ai
    ws_document
    ws_network
    ws_utils
    Qt6::Widgets
    Qt6::Network
    Qt6::Core
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -O2)
endif()

option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()
