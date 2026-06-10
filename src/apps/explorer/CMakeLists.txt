add_executable(explorer
    main.cpp

    AppShell.cpp

    CacheExplorer.cpp
    CacheTreeBuilder.cpp

    panels/CacheTreePanel.cpp
    panels/CacheInspectorPanel.cpp
    panels/CacheViewportPanel.cpp
)

target_include_directories(explorer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(explorer PRIVATE
    Eldoria::Data
    core
    platform
    rune_forge_render_next
    render
    ui
)

set_target_properties(explorer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin
)
