add_library(ws_ui STATIC
    MainWindow.cpp
    MainWindow.h
    panels/SidebarPanel.cpp
    panels/SidebarPanel.h
    panels/EditorPanel.cpp
    panels/EditorPanel.h
    panels/ChatPanel.cpp
    panels/ChatPanel.h
    widgets/ChatBubble.cpp
    widgets/ChatBubble.h
    widgets/MarkdownEditor.cpp
    widgets/MarkdownEditor.h
    themes/ThemeManager.cpp
    themes/ThemeManager.h
)

target_include_directories(ws_ui PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(ws_ui PUBLIC ws_ai ws_document ws_network ws_utils Qt6::Widgets Qt6::Core)
