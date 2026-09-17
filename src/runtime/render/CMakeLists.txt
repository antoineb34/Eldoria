add_library(eldoria_render STATIC
    RenderPipeline.cpp
    Renderer3D.cpp
    backend/software/SoftwareRenderBackend.cpp
    backend/opengl/OpenGLRenderBackend.cpp
    model/ModelManager.cpp
    animation/AnimationPlayer.cpp
    animation/ModelSkinGroups.cpp
    animation/ModelAnimator.cpp
    map/ClassicTerrainAppearance.cpp
    map/SceneLocationBuilder.cpp
    map/SceneLocationModelBuilder.cpp
    map/SceneTerrainBuilder.cpp
    backend/software/SoftwareMeshProjector.cpp
    backend/software/SoftwareTextureSampler.cpp
    backend/software/TriangleRasterizer.cpp
    backend/opengl/OpenGLApi.cpp
    camera/Projection.cpp
    scene/Transform.cpp
    texture/TextureManager.cpp

    backend/opengl/SimpleRenderer.cpp
)

add_library(Eldoria::Render ALIAS eldoria_render)

target_include_directories(eldoria_render PUBLIC
    ${PROJECT_SOURCE_DIR}/src/runtime
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_render PUBLIC

    eldoria_glad

    Eldoria::Assets

    Eldoria::Math

    SDL3::SDL3

    OpenGL::GL

)