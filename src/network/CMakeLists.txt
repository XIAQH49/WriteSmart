add_library(ws_network STATIC
    HttpClient.cpp
    HttpClient.h
    ApiConfig.cpp
    ApiConfig.h
    StreamHandler.cpp
    StreamHandler.h
)

target_include_directories(ws_network PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(ws_network PUBLIC ws_utils Qt6::Core Qt6::Network)
