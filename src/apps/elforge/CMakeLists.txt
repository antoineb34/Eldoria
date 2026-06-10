add_executable(elforge
    main.cpp

    AppShell.cpp
    theme/ImGuiTheme.cpp

    CacheExplorer.cpp
    CacheTreeBuilder.cpp

    panels/CacheTreePanel.cpp
    panels/CacheInspectorPanel.cpp
    panels/CacheViewportPanel.cpp
)

target_include_directories(elforge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(elforge PRIVATE
    Eldoria::Data
    platform
    platform_imgui
    render
)

set_target_properties(elforge PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin
)
