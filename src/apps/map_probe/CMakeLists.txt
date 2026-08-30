add_executable(map_probe
    main.cpp
)

target_link_libraries(map_probe PRIVATE
    Eldoria::Data
)

set_target_properties(map_probe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
