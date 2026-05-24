#pragma once

#include <cstdint>
#include <vector>

namespace rf::texture {

struct RgbColor {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
};

struct TextureMetadata {
    int xOffset = 0;
    int yOffset = 0;
    int width = 0;
    int height = 0;
    int type = 0;
};

struct TextureIndex {
    int canvasWidth = 0;
    int canvasHeight = 0;

    std::vector<RgbColor> palette;
    std::vector<TextureMetadata> textures;

    std::vector<std::uint8_t> rawData;
};

struct DecodedTexture {
    int width = 0;
    int height = 0;

    std::vector<std::uint8_t> pixels;
};

}
