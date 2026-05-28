add_executable(tool
    main.cpp

    StudioApplication.cpp

    workspace/cache/CacheTreeBuilder.cpp
    workspace/cache/CacheWorkspace.cpp

    panels/cache/CacheTreePanel.cpp
    panels/cache/CacheInspectorPanel.cpp
    panels/cache/CachePreviewPanel.cpp
)

target_include_directories(tool PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(tool PRIVATE
    core
    platform
    render
    ui
)

set_target_properties(tool PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin
)
