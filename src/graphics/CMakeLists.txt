add_library(eldoria_graphics STATIC
    map/SceneMapRenderModelBuilder.cpp

    map/TerrainBuilder.cpp
    map/ClassicTerrainLighting.cpp
    map/ClassicTerrainShape.cpp
    map/LocationBuilder.cpp
    map/LocationBatchBuilder.cpp
    map/LocationModelBuilder.cpp

    model/ModelBuilder.cpp
    model/ModelSystem.cpp

    texture/TextureBuilder.cpp
    texture/TextureSystem.cpp
)

add_library(
    Eldoria::Graphics
    ALIAS eldoria_graphics
)

target_include_directories(
    eldoria_graphics
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    eldoria_graphics
    PUBLIC
        Eldoria::World
        Eldoria::Assets
        Eldoria::Render
)
