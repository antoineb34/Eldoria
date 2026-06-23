add_library(eldoria_render STATIC
    RenderPipeline.cpp

    backend/software/SoftwareMeshProjector.cpp
    backend/software/SoftwareRenderBackend.cpp
    backend/software/TextureSampler.cpp
    backend/software/TriangleRasterizer.cpp

    camera/Projection.cpp

    scene/Transform.cpp
)

add_library(Eldoria::Render ALIAS eldoria_render)

target_include_directories(eldoria_render PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_render PUBLIC
    Eldoria::Math
    Eldoria::Graphics
    SDL3::SDL3
)
