add_library(platform_sdl STATIC
    sdl/Renderer3D.cpp
)

target_include_directories(platform_sdl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/sdl
)

target_link_libraries(platform_sdl PUBLIC
    core
    SDL3::SDL3
    OpenGL::GL
)
