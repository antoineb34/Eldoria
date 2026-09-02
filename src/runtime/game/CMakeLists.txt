add_library(eldoria_game INTERFACE)

add_library(Eldoria::Game ALIAS eldoria_game)

target_include_directories(eldoria_game INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_game INTERFACE
    Eldoria::Assets
    Eldoria::World
)
