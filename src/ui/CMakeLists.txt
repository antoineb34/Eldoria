add_library(ui STATIC
    imgui/ImGuiTheme.cpp
    ../../libs/imgui/imgui.cpp
    ../../libs/imgui/imgui_draw.cpp
    ../../libs/imgui/imgui_tables.cpp
    ../../libs/imgui/imgui_widgets.cpp
    ../../libs/imgui/backends/imgui_impl_sdl3.cpp
    ../../libs/imgui/backends/imgui_impl_sdlrenderer3.cpp
)

target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/imgui/backends
)

target_link_libraries(ui PUBLIC
    platform
    SDL3::SDL3
)