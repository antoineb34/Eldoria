add_executable(elclient
    main.cpp
    ElClientApp.cpp
)

target_include_directories(elclient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(elclient PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin
)
