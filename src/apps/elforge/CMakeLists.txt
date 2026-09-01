add_executable(elforge
    main.cpp

    AppShell.cpp
    AnimationDump.cpp
    AnimationRelations.cpp
    CacheExplorer.cpp
    CacheTreeBuilder.cpp
    FontPreviewBuilder.cpp
    IdentityKitPreviewBuilder.cpp
    ItemPreviewBuilder.cpp
    LocationPreviewBuilder.cpp
    MapPreview.cpp
    MapPreviewBuilder.cpp
    MidiExport.cpp
    NpcPreviewBuilder.cpp
    SpotAnimationPreviewBuilder.cpp

    panels/CacheInspectorPanel.cpp
    panels/CacheTreePanel.cpp
    panels/CacheViewportPanel.cpp
    panels/InterfaceViewPanel.cpp
    panels/MapGpuViewportRenderer.cpp
    panels/ModelViewPanel.cpp
    panels/TextureViewPanel.cpp
    panels/ViewportViewDrawer.cpp
)

target_include_directories(elforge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(elforge PRIVATE
    Eldoria::Data
    Eldoria::Platform
    Eldoria::PlatformImGui
    Eldoria::Render
)

set_target_properties(elforge PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
