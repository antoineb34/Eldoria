add_library(core STATIC
    io/ByteBuffer.cpp
    compression/Compression.cpp

    cache/Cache.cpp

    debug/ModelDebug.cpp

    assets/model/ModelLoader.cpp
    assets/model/ModelFileReader.cpp
    assets/model/ModelBuilder.cpp
    assets/model/decoder/VertexDecoder.cpp
    assets/model/decoder/FaceDecoder.cpp
    assets/model/decoder/TextureUVMappingDecoder.cpp

    assets/texture/TextureLoader.cpp
    assets/texture/TextureFileReader.cpp
    assets/texture/TextureBuilder.cpp
    assets/texture/decoder/TexturePixelDecoder.cpp
    assets/texture/decoder/TextureCanvasDecoder.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core PUBLIC
    BZip2::BZip2
    ZLIB::ZLIB
)
