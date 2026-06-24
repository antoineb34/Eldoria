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

    interface/InterfaceParser.cpp
    interface/InterfaceRepository.cpp

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
    definition/idk/IdentityKitDefinitionParser.cpp
    definition/idk/IdentityKitRepository.cpp
    definition/location/LocationDefinitionParser.cpp
    definition/location/LocationRepository.cpp
    definition/npc/NpcDefinitionParser.cpp
    definition/npc/NpcRepository.cpp
    definition/item/ItemDefinitionParser.cpp
    definition/item/ItemRepository.cpp
    definition/sequence/SequenceDefinitionParser.cpp
    definition/sequence/SequenceRepository.cpp
    definition/spot_animation/SpotAnimationDefinitionParser.cpp
    definition/spot_animation/SpotAnimationRepository.cpp
    definition/varp/VarpDefinitionParser.cpp
    definition/varp/VarpRepository.cpp
    definition/varbit/VarbitDefinitionParser.cpp
    definition/varbit/VarbitRepository.cpp
    definition/parameter/ParameterDefinitionParser.cpp
    definition/parameter/ParameterRepository.cpp
    definition/message/MessageDefinitionParser.cpp
    definition/message/MessageRepository.cpp
    definition/message_animation/MessageAnimationDefinitionParser.cpp
    definition/message_animation/MessageAnimationRepository.cpp
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
