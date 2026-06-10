add_library(eldoria_render STATIC
    RenderPipeline.cpp

    backend/software/SoftwareRenderBackend.cpp
    backend/software/TriangleRasterizer.cpp

    camera/Projection.cpp

    color/Color.cpp

    geometry/MeshProjector.cpp

    material/MaterialResolver.cpp
    material/TextureSampler.cpp

    pipeline/DepthSorter.cpp
    pipeline/FaceAssembler.cpp
    pipeline/VisibilityStage.cpp

    scene/Transform.cpp
)

add_library(Eldoria::Render ALIAS eldoria_render)

target_include_directories(eldoria_render PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_render PUBLIC
    Eldoria::Data
    SDL3::SDL3
)
