add_library(eldoria_data STATIC
    cache/Cache.cpp
    cache/Archive.cpp
    cache/ArchiveReader.cpp
    cache/versionlist/VersionListReader.cpp
)

add_library(Eldoria::Data ALIAS eldoria_data)

target_include_directories(eldoria_data PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_data PUBLIC
    BZip2::BZip2
)
