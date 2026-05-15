add_library(ws_ai STATIC
    AIProvider.h
    AISession.cpp
    AISession.h
    PromptTemplate.cpp
    PromptTemplate.h
    providers/OpenAIProvider.cpp
    providers/OpenAIProvider.h
    providers/ClaudeProvider.cpp
    providers/ClaudeProvider.h
    providers/CustomProvider.cpp
    providers/CustomProvider.h
)

target_include_directories(ws_ai PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(ws_ai PUBLIC ws_network ws_utils Qt6::Core Qt6::Network)
