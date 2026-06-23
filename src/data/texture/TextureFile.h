#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace eld::texture {

enum class TexturePixelOrder : std::uint8_t {
    RowMajor = 0,
    ColumnMajor = 1
};

struct TextureMetadata {
    std::uint16_t indexOffset = 0;

    std::uint16_t canvasWidth = 0;
    std::uint16_t canvasHeight = 0;

    std::uint8_t offsetX = 0;
    std::uint8_t offsetY = 0;

    std::uint16_t width = 0;
    std::uint16_t height = 0;

    TexturePixelOrder pixelOrder =
        TexturePixelOrder::RowMajor;
};

struct TextureColor {
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;
};

struct TexturePixel {
    std::size_t sourceOffset = 0;
    std::uint8_t paletteIndex = 0;
};

struct TextureFile {
    std::vector<std::uint8_t> dataPayload;
    std::vector<std::uint8_t> indexPayload;

    TextureMetadata metadata;
    std::vector<TextureColor> palette;
    std::vector<TexturePixel> pixels;
};

}
