add_executable(tool
    main.cpp
    ToolApplication.cpp
    modes/ModelViewerMode.cpp
    modes/CacheExplorerMode.cpp

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
    ../../core/render/DepthBuffer.cpp

    ../../core/cache/ArchiveDecoder.cpp
    ../../core/cache/ArchiveFileTable.cpp
    ../../core/cache/NameHash.cpp
    ../../core/cache/ArchiveFileExtractor.cpp
    ../../core/cache/ConfigArchiveLoader.cpp

    ../../../libs/imgui/imgui.cpp
    ../../../libs/imgui/imgui_draw.cpp
    ../../../libs/imgui/imgui_tables.cpp
    ../../../libs/imgui/imgui_widgets.cpp
    ../../../libs/imgui/backends/imgui_impl_sdl3.cpp
    ../../../libs/imgui/backends/imgui_impl_sdlrenderer3.cpp
)

target_link_libraries(tool PRIVATE
    SDL3::SDL3
    z
    BZip2::BZip2
)

target_include_directories(tool PRIVATE
    ../../../libs/imgui
    ../../../libs/imgui/backends
)

set_target_properties(tool PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin
)
