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

# Qt 配置
find_package(Qt6 REQUIRED COMPONENTS Widgets Network Core)

# 源文件
set(SOURCES
    src/main.cpp

    # App
    src/app/Application.cpp
    src/app/Config.cpp

    # UI
    src/ui/MainWindow.cpp
    src/ui/panels/SidebarPanel.cpp
    src/ui/panels/EditorPanel.cpp
    src/ui/panels/ChatPanel.cpp
    src/ui/widgets/ChatBubble.cpp
    src/ui/widgets/MarkdownEditor.cpp
    src/ui/themes/ThemeManager.cpp

    # Core - Document
    src/core/document/Document.cpp
    src/core/document/DocumentManager.cpp
    src/core/document/Paragraph.cpp

    # Core - AI
    src/core/ai/AISession.cpp
    src/core/ai/PromptTemplate.cpp
    src/core/ai/providers/OpenAIProvider.cpp
    src/core/ai/providers/ClaudeProvider.cpp
    src/core/ai/providers/CustomProvider.cpp

    # Core - Plugin
    src/core/plugin/PluginManager.cpp

    # Network
    src/network/HttpClient.cpp
    src/network/ApiConfig.cpp
    src/network/StreamHandler.cpp

    # Utils
    src/utils/JsonHelper.cpp
    src/utils/Logger.cpp
    src/utils/StringUtils.cpp
)

set(HEADERS
    src/app/Application.h
    src/app/Config.h

    src/ui/MainWindow.h
    src/ui/panels/SidebarPanel.h
    src/ui/panels/EditorPanel.h
    src/ui/panels/ChatPanel.h
    src/ui/widgets/ChatBubble.h
    src/ui/widgets/MarkdownEditor.h
    src/ui/themes/ThemeManager.h

    src/core/document/Document.h
    src/core/document/DocumentManager.h
    src/core/document/Paragraph.h

    src/core/ai/AIProvider.h
    src/core/ai/AISession.h
    src/core/ai/PromptTemplate.h
    src/core/ai/providers/OpenAIProvider.h
    src/core/ai/providers/ClaudeProvider.h
    src/core/ai/providers/CustomProvider.h

    src/core/plugin/PluginInterface.h
    src/core/plugin/PluginManager.h

    src/network/HttpClient.h
    src/network/ApiConfig.h
    src/network/StreamHandler.h

    src/utils/JsonHelper.h
    src/utils/Logger.h
    src/utils/StringUtils.h
)

set(RESOURCES
    resources/resources.qrc
)

# 可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/document
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/ai
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/plugin
    ${CMAKE_CURRENT_SOURCE_DIR}/src/network
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Core
)

# 编译选项
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -O2)
endif()

# 测试 (可选)
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()
