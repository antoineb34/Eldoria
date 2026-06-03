add_library(rune_forge_render_next
    RenderPipeline.cpp

    geometry/MeshProjector.cpp

    pipeline/FaceAssembler.cpp
    pipeline/VisibilityStage.cpp
    pipeline/DepthSorter.cpp

    material/MaterialResolver.cpp

    backend/software/SoftwareRenderBackend.cpp
    backend/software/TriangleRasterizer.cpp
)

target_include_directories(rune_forge_render_next
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
