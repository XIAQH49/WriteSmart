add_library(ws_app STATIC
    Application.cpp
    Application.h
)

target_include_directories(ws_app PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(ws_app PUBLIC ws_ui ws_plugin ws_ai ws_document ws_network ws_utils Qt6::Core Qt6::Widgets)
