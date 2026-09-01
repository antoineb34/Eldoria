add_library(eldoria_platform STATIC
    sdl/SdlContext.cpp
    sdl/SdlOpenGLContext.cpp
)

add_library(Eldoria::Platform ALIAS eldoria_platform)

target_include_directories(eldoria_platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_platform PUBLIC
    SDL3::SDL3
)

add_subdirectory(imgui)
