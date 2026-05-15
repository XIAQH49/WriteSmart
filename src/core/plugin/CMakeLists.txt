add_library(ws_plugin STATIC
    PluginInterface.h
    PluginManager.cpp
    PluginManager.h
)

target_include_directories(ws_plugin PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(ws_plugin PUBLIC ws_ai ws_document ws_utils Qt6::Core)
