add_library(eldoria_data STATIC
    binary/ByteBuffer.cpp
    compression/Compression.cpp

    cache/Cache.cpp
    cache/CacheFileDetails.cpp
    cache/Archive.cpp
    cache/ArchiveReader.cpp
    cache/versionlist/VersionListReader.cpp

    model/ModelLoader.cpp
    model/ModelFileReader.cpp
    model/ModelBuilder.cpp
    model/decoder/VertexDecoder.cpp
    model/decoder/FaceDecoder.cpp
    model/decoder/TextureUVMappingDecoder.cpp

    texture/TextureLoader.cpp
    texture/TextureFileReader.cpp
    texture/TextureBuilder.cpp
    texture/decoder/TexturePixelDecoder.cpp
    texture/decoder/TextureCanvasDecoder.cpp
)

add_library(Eldoria::Data ALIAS eldoria_data)

target_include_directories(eldoria_data PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_data PUBLIC
    BZip2::BZip2
    ZLIB::ZLIB
)
