add_library(eldoria_assets STATIC
    AnimationFrameTable.cpp
    content/animation/AnimationPresentationCatalog.cpp
    decoders/AnimationDecoder.cpp
    repositories/AnimationRepository.cpp
    InterfaceComposer.cpp
    decoders/WidgetDecoder.cpp
    repositories/WidgetRepository.cpp
    content/map/MapFileReader.cpp
    content/map/MapIndexParser.cpp
    content/map/MapRepository.cpp
    content/map/TerrainDecoder.cpp
    content/map/LocationSpawnDecoder.cpp
    decoders/MidiDecoder.cpp
    repositories/MidiRepository.cpp
    decoders/ModelDecoder.cpp
    repositories/ModelRepository.cpp
    decoders/ImageDecoder.cpp
    repositories/ImageRepository.cpp
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
    decoders/SequenceDecoder.cpp
    repositories/SequenceRepository.cpp
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
    repositories/TextureRepository.cpp
    repositories/SpriteRepository.cpp
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


# Public asset-access facade.


target_sources(
    eldoria_assets
    PRIVATE
        ResourceProvider.cpp
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
