add_library(eldoria_network INTERFACE)

add_library(Eldoria::Network ALIAS eldoria_network)

target_include_directories(eldoria_network INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_network INTERFACE
    Eldoria::Data
    Eldoria::World
)
