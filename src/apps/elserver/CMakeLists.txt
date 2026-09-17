add_executable(elserver
    main.cpp

    app/ElServerApp.cpp
    app/ServerRunLoop.cpp
)

target_include_directories(elserver PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(elserver PRIVATE
    Eldoria::Assets
    Eldoria::Game
    Eldoria::Network
    Eldoria::World
)

set_target_properties(elserver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
