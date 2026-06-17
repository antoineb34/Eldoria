add_library(eldoria_data STATIC
    binary/ByteReader.cpp
    binary/ByteWriter.cpp
    binary/Compression.cpp

    cache/Cache.cpp
    cache/File.cpp
    cache/Payload.cpp
    cache/Reader.cpp
    cache/Store.cpp
    cache/Writer.cpp

    cache_legacy/Archive.cpp
    cache_legacy/ArchiveReader.cpp
    cache_legacy/Cache.cpp

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
