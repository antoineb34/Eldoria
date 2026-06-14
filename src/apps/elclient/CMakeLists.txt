add_executable(elclient
    main.cpp

    ElClientApp.cpp
    ClientRenderContext.cpp
)

target_include_directories(elclient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(elclient PRIVATE
    Eldoria::Platform
    Eldoria::Render
)

set_target_properties(elclient PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
