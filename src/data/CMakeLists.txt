add_library(eldoria_data INTERFACE)

add_library(Eldoria::Data ALIAS eldoria_data)

target_include_directories(eldoria_data INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
