add_executable(explorer
    main.cpp

    AppShell.cpp
    theme/ImGuiTheme.cpp

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
    platform
    platform_imgui
    render
)

set_target_properties(explorer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin
)
