add_library(eldoria_host STATIC
    sdl/SdlContext.cpp
    sdl/SdlOpenGLContext.cpp
)

add_library(Eldoria::Host ALIAS eldoria_host)

target_include_directories(eldoria_host PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_host PUBLIC
    SDL3::SDL3
)

add_subdirectory(imgui)
