add_library(eldoria_storage STATIC
    cache/Cache.cpp
    cache/File.cpp
    cache/Payload.cpp
    cache/Reader.cpp
    cache/Store.cpp
    cache/Writer.cpp
    archive/Archive.cpp
    archive/ArchiveParser.cpp
)

add_library(Eldoria::Storage ALIAS eldoria_storage)

target_include_directories(eldoria_storage PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_storage PUBLIC
    Eldoria::Binary
)
