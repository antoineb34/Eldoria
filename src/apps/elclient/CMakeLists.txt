add_executable(elclient
    main.cpp
)

target_include_directories(elclient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(elclient PRIVATE
    Eldoria::Host
    Eldoria::Render
    Eldoria::Assets
)

set_target_properties(elclient PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
