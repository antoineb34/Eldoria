add_library(render STATIC
    model/ModelRenderer.cpp

    software/camera/Projection.cpp

    software/color/Color.cpp

    software/raster/TriangleRasterizer.cpp
)

target_include_directories(render PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(render PUBLIC
    core
    SDL3::SDL3
)
