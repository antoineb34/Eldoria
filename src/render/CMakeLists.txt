add_library(render STATIC
    model/ModelRenderer.cpp
    model/RenderMeshBuilder.cpp
    order/FaceOrderer.cpp

    camera/Projection.cpp
    color/Color.cpp
    software/raster/TriangleRasterizer.cpp

    RenderPipeline.cpp

    scene/Transform.cpp

    geometry/MeshProjector.cpp

    pipeline/FaceAssembler.cpp
    pipeline/VisibilityStage.cpp
    pipeline/DepthSorter.cpp

    material/MaterialResolver.cpp
    material/TextureSampler.cpp

    backend/software/SoftwareRenderBackend.cpp
    backend/software/TriangleRasterizer.cpp
)

target_include_directories(render PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(render PUBLIC
    Eldoria::Data
    SDL3::SDL3
)
