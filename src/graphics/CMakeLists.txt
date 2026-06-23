add_library(eldoria_graphics STATIC
    texture/TextureConverter.cpp
    texture/TextureRegistry.cpp
    texture/TextureResolver.cpp
)

add_library(Eldoria::Graphics ALIAS eldoria_graphics)

target_include_directories(eldoria_graphics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_graphics PUBLIC
    Eldoria::Data
)
