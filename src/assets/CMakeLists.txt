add_library(eldoria_assets STATIC
    AssetManager.cpp
    animation/AnimationDecoder.cpp
    animation/AnimationLoader.cpp
    animation/AnimationFrameTable.cpp
    content/animation/AnimationPresentationCatalog.cpp
    interface/WidgetDecoder.cpp
    interface/WidgetAssembler.cpp
    interface/WidgetLoader.cpp
    map/MapIndexDecoder.cpp
    map/TerrainDecoder.cpp
    map/LocationSpawnDecoder.cpp
    map/MapAssembler.cpp
    map/MapLoader.cpp
    midi/MidiDecoder.cpp
    midi/MidiLoader.cpp
    model/ModelDecoder.cpp
    model/ModelAssembler.cpp
    model/ModelLoader.cpp
    image/ImageDecoder.cpp
    title/TitleLoader.cpp
    font/FontDecoder.cpp
    font/FontLoader.cpp
    floor/FloorDecoder.cpp
    floor/FloorLoader.cpp
    identity_kit/IdentityKitDecoder.cpp
    identity_kit/IdentityKitLoader.cpp
    location/LocationDecoder.cpp
    location/LocationLoader.cpp
    npc/NpcDecoder.cpp
    npc/NpcLoader.cpp
    item/ItemDecoder.cpp
    item/ItemLoader.cpp
    sequence/SequenceDecoder.cpp
    sequence/SequenceAssembler.cpp
    sequence/SequenceLoader.cpp
    spot_animation/SpotAnimationDecoder.cpp
    spot_animation/SpotAnimationAssembler.cpp
    spot_animation/SpotAnimationLoader.cpp
    varp/VarpDecoder.cpp
    varp/VarpLoader.cpp
    varbit/VarbitDecoder.cpp
    varbit/VarbitLoader.cpp
    parameter/ParameterDecoder.cpp
    parameter/ParameterLoader.cpp
    message/MessageDecoder.cpp
    message/MessageLoader.cpp
    message_animation/MessageAnimationDecoder.cpp
    message_animation/MessageAnimationLoader.cpp
    texture/TextureAssembler.cpp
    texture/TextureLoader.cpp
    sprite/SpriteAssembler.cpp
    sprite/SpriteLoader.cpp
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
