add_executable(tool
    main.cpp
    ToolApplication.cpp

    modes/ModelViewerMode.cpp
    modes/CacheExplorerMode.cpp
)

target_include_directories(tool PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(tool PRIVATE
    core
    ui
)

set_target_properties(tool PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin
)
