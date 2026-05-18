find_package(SDL3 REQUIRED CONFIG)

add_executable(client
    main.cpp
    render/Renderer3D.cpp
)

target_include_directories(client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/render
)

target_link_libraries(client PRIVATE
    core
    SDL3::SDL3
    OpenGL::GL
)
