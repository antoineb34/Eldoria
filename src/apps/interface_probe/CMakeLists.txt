add_executable(interface_probe
    main.cpp
)

target_include_directories(interface_probe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(interface_probe PRIVATE
    Eldoria::Data
    Eldoria::Platform
    Eldoria::PlatformImGui
    Eldoria::Graphics
    Eldoria::Render
    Eldoria::Math
)

set_target_properties(interface_probe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
