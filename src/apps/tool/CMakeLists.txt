add_executable(tool
    main.cpp
    ModelViewerTool.cpp

    ../../core/cache/CacheStore.cpp

    ../../core/io/ByteBuffer.cpp
    ../../core/io/Compression.cpp

    ../../core/model/ModelFooter.cpp
    ../../core/model/ModelLayout.cpp
    ../../core/model/VertexDecoder.cpp
    ../../core/model/FaceDecoder.cpp

    ../../core/platform/SdlContext.cpp

    ../../core/render/Projection.cpp
    ../../core/render/WireframeRenderer.cpp
    ../../core/debug/ModelDebug.cpp
    ../../core/render/Color.cpp
    ../../core/render/TriangleRasterizer.cpp
)

target_link_libraries(tool PRIVATE
    SDL3::SDL3
    z
)

set_target_properties(tool PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin
)
