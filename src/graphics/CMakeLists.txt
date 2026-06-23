add_library(eldoria_graphics STATIC
    model/ModelRegistry.cpp
    model/ModelResolver.cpp
    model/RenderModelAssembler.cpp

    texture/TextureConverter.cpp
    texture/TextureRegistry.cpp
    texture/TextureResolver.cpp

    GraphicsResources.cpp
)

add_library(Eldoria::Graphics ALIAS eldoria_graphics)

target_include_directories(eldoria_graphics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_graphics PUBLIC
    Eldoria::Data
    Eldoria::Math
)
