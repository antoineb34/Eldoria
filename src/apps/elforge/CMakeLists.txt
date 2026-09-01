add_executable(elforge
    main.cpp

    app/AppShell.cpp
    ui/ElForgeTheme.cpp
    ui/IconButton.cpp
    ui/Carousel.cpp
    dump/AssetDumpWriter.cpp
    dump/AnimationDumper.cpp
    export/AnimationExporter.cpp
    inspection/AnimationInspector.cpp
    explorer/CacheExplorer.cpp
    explorer/CacheExplorerAnimation.cpp
    explorer/CacheExplorerItem.cpp
    explorer/CacheExplorerSelection.cpp
    explorer/tree/CacheTreeBuilder.cpp
    views/animation/AnimationViewPanel.cpp
    views/animation/AnimationViewOverlay.cpp
    views/floor/FloorView.cpp
    views/font/FontView.cpp
    views/identity_kit/IdentityKitView.cpp
    inspection/InterfaceInspector.cpp
    views/item/ItemAnimationPresets.cpp
    views/item/ItemView.cpp
    views/location/LocationView.cpp
    views/map/MapViewState.cpp
    views/map/MapView.cpp
    views/midi/MidiView.cpp
    views/midi/MidiViewPanel.cpp
    export/MidiExporter.cpp
    views/npc/NpcView.cpp
    views/spot_animation/SpotAnimationView.cpp

    explorer/CacheInspectorPanel.cpp
    explorer/tree/CacheTreePanel.cpp
    viewport/ViewportPanel.cpp
    viewport/ViewportSurface.cpp
    views/interface/InterfaceViewPanel.cpp
    views/map/MapViewSurface.cpp
    views/model/ModelViewPanel.cpp
    views/texture/TextureViewPanel.cpp
    viewport/ViewportControlsPanel.cpp
)

target_include_directories(elforge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(elforge PRIVATE
    Eldoria::Audio
    Eldoria::Data
    Eldoria::Platform
    Eldoria::PlatformImGui
    Eldoria::Render
)

set_target_properties(elforge PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
