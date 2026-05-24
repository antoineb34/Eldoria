add_library(render STATIC
    software/Projection.cpp
    software/WireframeRenderer.cpp
    software/Color.cpp
    software/TriangleRasterizer.cpp
    software/DepthBuffer.cpp
)

target_include_directories(render PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(render PUBLIC
    core
    SDL3::SDL3
)