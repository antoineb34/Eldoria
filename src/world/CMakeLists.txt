add_library(eldoria_world INTERFACE)

add_library(Eldoria::World ALIAS eldoria_world)

target_include_directories(eldoria_world INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_world INTERFACE
    Eldoria::Data
)
