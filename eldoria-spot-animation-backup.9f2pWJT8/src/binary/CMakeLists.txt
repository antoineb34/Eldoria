add_library(eldoria_binary STATIC
    ByteReader.cpp
    ByteWriter.cpp
    Compression.cpp
)

add_library(Eldoria::Binary ALIAS eldoria_binary)

target_include_directories(eldoria_binary PUBLIC
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(eldoria_binary PUBLIC
    BZip2::BZip2
    ZLIB::ZLIB
)
