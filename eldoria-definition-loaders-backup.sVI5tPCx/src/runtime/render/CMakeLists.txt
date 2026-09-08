add_library(eldoria_render STATIC
    animation/AnimationPlayer.cpp
    animation/ModelSkinGroups.cpp
    animation/ModelAnimator.cpp
    model/ModelRegistry.cpp
    model/ModelResolver.cpp
    model/RenderModelAssembler.cpp
    map/ClassicTerrainAppearance.cpp
    map/SceneLocationBuilder.cpp
    map/SceneLocationModelBuilder.cpp
    map/SceneMapRenderModelBuilder.cpp
    map/SceneTerrainBuilder.cpp
    texture/TextureConverter.cpp
    texture/TextureRegistry.cpp
    texture/TextureResolver.cpp
    GraphicsResources.cpp
    RenderPipeline.cpp
    backend/software/SoftwareMeshProjector.cpp
    backend/software/SoftwareRenderBackend.cpp
    backend/software/TextureSampler.cpp
    backend/software/TriangleRasterizer.cpp
    backend/opengl/OpenGLApi.cpp
    backend/opengl/OpenGLRenderBackend.cpp
    camera/Projection.cpp
    scene/Transform.cpp
)

add_library(Eldoria::Render ALIAS eldoria_render)

target_include_directories(eldoria_render PUBLIC
    ${PROJECT_SOURCE_DIR}/src/runtime
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_render PUBLIC
    Eldoria::Assets
    Eldoria::Math
    SDL3::SDL3
)
