add_library(eldoria_host_imgui STATIC
    ImGuiTheme.cpp

    ../../../libs/imgui/imgui.cpp
    ../../../libs/imgui/imgui_draw.cpp
    ../../../libs/imgui/imgui_tables.cpp
    ../../../libs/imgui/imgui_widgets.cpp

    ../../../libs/imgui/backends/imgui_impl_sdl3.cpp
    ../../../libs/imgui/backends/imgui_impl_sdlrenderer3.cpp
)

add_library(Eldoria::HostImGui ALIAS eldoria_host_imgui)

target_include_directories(eldoria_host_imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/imgui/backends
)

target_link_libraries(eldoria_host_imgui PUBLIC
    Eldoria::Host
    SDL3::SDL3
)
