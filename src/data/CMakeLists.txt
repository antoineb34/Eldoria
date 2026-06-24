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

    archive/Archive.cpp
    archive/ArchiveParser.cpp

    model/ModelDecoder.cpp
    model/ModelFileParser.cpp
    model/ModelRepository.cpp

    image/IndexedImageDecoder.cpp
    image/JpegDecoder.cpp
    image/JpegRepository.cpp
    font/FontFileParser.cpp
    font/FontDecoder.cpp
    font/FontRepository.cpp
    definition/DefinitionTableParser.cpp
    definition/DefinitionRepository.cpp
    definition/floor/FloorDefinitionParser.cpp
    definition/floor/FloorRepository.cpp
    image/IndexedImageFileParser.cpp

    texture/TextureRepository.cpp
    sprite/SpriteRepository.cpp
)

add_library(Eldoria::Data ALIAS eldoria_data)

target_include_directories(eldoria_data PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_data PUBLIC
    BZip2::BZip2
    ZLIB::ZLIB
    JPEG::JPEG
)
