add_library(eldoria_data STATIC
    binary/ByteBuffer.cpp
    binary/Compression.cpp

    cache/Archive.cpp
    cache/ArchiveReader.cpp
    cache/Cache.cpp

    model/ModelBuilder.cpp
    model/ModelFileReader.cpp
    model/ModelLoader.cpp
    model/decoder/FaceDecoder.cpp
    model/decoder/TextureUVMappingDecoder.cpp
    model/decoder/VertexDecoder.cpp

    texture/TextureBuilder.cpp
    texture/TextureFileReader.cpp
    texture/TextureLoader.cpp
    texture/decoder/TextureCanvasDecoder.cpp
    texture/decoder/TexturePixelDecoder.cpp
)

add_library(Eldoria::Data ALIAS eldoria_data)

target_include_directories(eldoria_data PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_data PUBLIC
    BZip2::BZip2
    ZLIB::ZLIB
)
