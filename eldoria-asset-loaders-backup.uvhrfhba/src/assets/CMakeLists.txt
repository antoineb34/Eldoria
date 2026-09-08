add_library(eldoria_assets STATIC
    AssetManager.cpp
    animation/AnimationDecoder.cpp
    animation/AnimationPipeline.cpp
    animation/AnimationFrameTable.cpp
    content/animation/AnimationPresentationCatalog.cpp
    InterfaceComposer.cpp
    decoders/WidgetDecoder.cpp
    repositories/WidgetRepository.cpp
    decoders/MapIndexDecoder.cpp
    decoders/TerrainDecoder.cpp
    decoders/LocationSpawnDecoder.cpp
    repositories/MapRepository.cpp
    decoders/MidiDecoder.cpp
    repositories/MidiRepository.cpp
    model/ModelDecoder.cpp
    model/ModelAssembler.cpp
    model/ModelPipeline.cpp
    image/ImageDecoder.cpp
    title/TitlePipeline.cpp
    decoders/FontDecoder.cpp
    repositories/FontRepository.cpp
    decoders/FloorDecoder.cpp
    repositories/FloorRepository.cpp
    decoders/IdentityKitDecoder.cpp
    repositories/IdentityKitRepository.cpp
    decoders/LocationDecoder.cpp
    repositories/LocationRepository.cpp
    decoders/NpcDecoder.cpp
    repositories/NpcRepository.cpp
    decoders/ItemDecoder.cpp
    repositories/ItemRepository.cpp
    sequence/SequenceDecoder.cpp
    sequence/SequenceAssembler.cpp
    sequence/SequencePipeline.cpp
    decoders/SpotAnimationDecoder.cpp
    repositories/SpotAnimationRepository.cpp
    decoders/VarpDecoder.cpp
    repositories/VarpRepository.cpp
    decoders/VarbitDecoder.cpp
    repositories/VarbitRepository.cpp
    decoders/ParameterDecoder.cpp
    repositories/ParameterRepository.cpp
    decoders/MessageDecoder.cpp
    repositories/MessageRepository.cpp
    decoders/MessageAnimationDecoder.cpp
    repositories/MessageAnimationRepository.cpp
    texture/TextureAssembler.cpp
    texture/TexturePipeline.cpp
    sprite/SpriteAssembler.cpp
    sprite/SpritePipeline.cpp
)

add_library(Eldoria::Assets ALIAS eldoria_assets)

target_include_directories(eldoria_assets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
    ${CMAKE_CURRENT_SOURCE_DIR}/content
)

target_link_libraries(eldoria_assets PUBLIC
    Eldoria::Storage
)

# Eldoria MIDI dependency

target_include_directories(
    eldoria_assets
    PRIVATE
        ${midifile_SOURCE_DIR}/include
)

target_link_libraries(
    eldoria_assets
    PRIVATE
        midifile
)
