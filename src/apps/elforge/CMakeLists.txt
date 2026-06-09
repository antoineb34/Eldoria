add_executable(elforge
    main.cpp
    ElForgeApplication.cpp
    panels/cache/CacheTreeBuilder.cpp
    panels/cache/CacheTreePanel.cpp
    panels/cache/CacheInspectorPanel.cpp
    panels/cache/CacheViewportPanel.cpp
)

target_include_directories(elforge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(elforge PRIVATE
    core
    platform
    rune_forge_render_next
    render
    ui
)

set_target_properties(elforge PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin
)
