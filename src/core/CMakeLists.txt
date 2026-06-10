add_library(core STATIC
    assets/texture/TextureLoader.cpp
    assets/texture/TextureFileReader.cpp
    assets/texture/TextureBuilder.cpp
    assets/texture/decoder/TexturePixelDecoder.cpp
    assets/texture/decoder/TextureCanvasDecoder.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core PUBLIC
    Eldoria::Data
)
