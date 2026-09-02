add_library(eldoria_assets STATIC
    content/animation_presentation/AnimationPresentationCatalog.cpp
    resources/animation/AnimationFrameIndex.cpp
    resources/animation/AnimationFileParser.cpp
    resources/animation/AnimationDecoder.cpp
    resources/animation/AnimationRepository.cpp
    content/interface/InterfaceFileParser.cpp
    content/interface/InterfaceRepository.cpp
    content/map/MapFileReader.cpp
    content/map/MapLoader.cpp
    content/map/decoder/TerrainDecoder.cpp
    content/map/decoder/LocationSpawnDecoder.cpp
    resources/midi/MidiFileParser.cpp
    resources/midi/MidiRepository.cpp
    resources/model/ModelDecoder.cpp
    resources/model/ModelFileParser.cpp
    resources/model/ModelRepository.cpp
    resources/image/IndexedImageDecoder.cpp
    resources/image/JpegDecoder.cpp
    resources/image/JpegRepository.cpp
    resources/font/FontFileParser.cpp
    resources/font/FontDecoder.cpp
    resources/font/FontRepository.cpp
    resources/definition/DefinitionTableParser.cpp
    resources/definition/DefinitionRepository.cpp
    content/floor/FloorDefinitionParser.cpp
    content/floor/FloorRepository.cpp
    content/identity_kit/IdentityKitDefinitionParser.cpp
    content/identity_kit/IdentityKitRepository.cpp
    content/location/LocationDefinitionParser.cpp
    content/location/LocationRepository.cpp
    content/npc/NpcDefinitionParser.cpp
    content/npc/NpcRepository.cpp
    content/item/ItemDefinitionParser.cpp
    content/item/ItemRepository.cpp
    resources/sequence/SequenceDefinitionParser.cpp
    resources/sequence/SequenceRepository.cpp
    content/spot_animation/SpotAnimationDefinitionParser.cpp
    content/spot_animation/SpotAnimationRepository.cpp
    content/varp/VarpDefinitionParser.cpp
    content/varp/VarpRepository.cpp
    content/varbit/VarbitDefinitionParser.cpp
    content/varbit/VarbitRepository.cpp
    content/parameter/ParameterDefinitionParser.cpp
    content/parameter/ParameterRepository.cpp
    content/message/MessageDefinitionParser.cpp
    content/message/MessageRepository.cpp
    content/message_animation/MessageAnimationDefinitionParser.cpp
    content/message_animation/MessageAnimationRepository.cpp
    resources/image/IndexedImageFileParser.cpp
    resources/texture/TextureRepository.cpp
    resources/sprite/SpriteRepository.cpp
)

add_library(Eldoria::Assets ALIAS eldoria_assets)

target_include_directories(eldoria_assets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
    ${CMAKE_CURRENT_SOURCE_DIR}/content
)

target_link_libraries(eldoria_assets PUBLIC
    Eldoria::Storage
    JPEG::JPEG
)
