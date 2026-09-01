add_executable(gpu_probe
    main.cpp
)

target_link_libraries(gpu_probe PRIVATE
    Eldoria::Data
    Eldoria::Platform
    Eldoria::Graphics
    Eldoria::Render
)

set_target_properties(gpu_probe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
