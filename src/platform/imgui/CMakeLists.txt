add_library(platform_imgui STATIC
    ../../../libs/imgui/imgui.cpp
    ../../../libs/imgui/imgui_draw.cpp
    ../../../libs/imgui/imgui_tables.cpp
    ../../../libs/imgui/imgui_widgets.cpp
    ../../../libs/imgui/backends/imgui_impl_sdl3.cpp
    ../../../libs/imgui/backends/imgui_impl_sdlrenderer3.cpp
)

add_library(Eldoria::PlatformImGui ALIAS platform_imgui)

target_include_directories(platform_imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/imgui/backends
)

target_link_libraries(platform_imgui PUBLIC
    platform
    SDL3::SDL3
)
