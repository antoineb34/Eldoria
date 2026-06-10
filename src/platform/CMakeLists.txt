add_library(platform STATIC
    sdl/SdlContext.cpp
)

target_include_directories(platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(platform PUBLIC
    SDL3::SDL3
)

add_subdirectory(imgui)
