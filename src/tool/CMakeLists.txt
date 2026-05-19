add_executable(tool
    main.cpp
    editing/AssetEditor.cpp
)

target_link_libraries(tool PRIVATE core platform_sdl)

target_include_directories(tool PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
