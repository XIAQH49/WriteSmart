add_library(ws_utils STATIC
    JsonHelper.cpp
    JsonHelper.h
    Logger.cpp
    Logger.h
    StringUtils.cpp
    StringUtils.h
    Config.cpp
    Config.h
)

target_include_directories(ws_utils PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(ws_utils PUBLIC Qt6::Core)
