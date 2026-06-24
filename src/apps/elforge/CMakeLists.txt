add_executable(elforge
    main.cpp

    AppShell.cpp
    CacheExplorer.cpp
    CacheTreeBuilder.cpp
    FontPreviewBuilder.cpp
    IdentityKitPreviewBuilder.cpp

    panels/CacheInspectorPanel.cpp
    panels/CacheTreePanel.cpp
    panels/CacheViewportPanel.cpp
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
