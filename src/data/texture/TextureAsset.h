#pragma once

#include <cstdint>
#include <vector>

namespace rf::texture {

struct RgbaColor {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};

struct RgbColor {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

struct TexturePalette {
    std::vector<RgbColor> colors;
};

struct TextureMetadata {
    int canvasWidth = 0;
    int canvasHeight = 0;

    int xOffset = 0;
    int yOffset = 0;

    int width = 0;
    int height = 0;

    int type = 0;
};

struct TextureAsset {
    TextureMetadata metadata;

    TexturePalette palette;

    std::vector<RgbaColor> pixels;
};

}
