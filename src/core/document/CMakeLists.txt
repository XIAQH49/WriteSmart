add_library(ws_document STATIC
    Document.cpp
    Document.h
    DocumentManager.cpp
    DocumentManager.h
    Paragraph.cpp
    Paragraph.h
)

target_include_directories(ws_document PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(ws_document PUBLIC ws_utils Qt6::Core)
