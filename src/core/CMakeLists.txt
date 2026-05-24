add_library(core STATIC
    io/ByteBuffer.cpp
    io/Compression.cpp

    cache/CacheStore.cpp
    cache/ArchiveDecoder.cpp
    cache/ArchiveFileTable.cpp
    cache/NameHash.cpp
    cache/ArchiveFileExtractor.cpp
    cache/ConfigArchiveLoader.cpp

    codecs/model/ModelFooter.cpp
    codecs/model/ModelLayout.cpp
    codecs/model/VertexDecoder.cpp
    codecs/model/FaceDecoder.cpp
    codecs/model/ModelDecoder.cpp
    codecs/model/TextureTriangleDecoder.cpp

    debug/ModelDebug.cpp

    codecs/texture/TextureDecoder.cpp
    codecs/texture/TextureIndex.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core PUBLIC
    BZip2::BZip2
    ZLIB::ZLIB
)