#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace eld::image {

enum class IndexedImagePixelOrder : std::uint8_t {
    RowMajor = 0,
    ColumnMajor = 1
};

struct IndexedImageMetadata {
    std::uint16_t indexOffset = 0;

    std::uint16_t canvasWidth = 0;
    std::uint16_t canvasHeight = 0;

    std::uint8_t offsetX = 0;
    std::uint8_t offsetY = 0;

    std::uint16_t width = 0;
    std::uint16_t height = 0;

    std::uint16_t frameId = 0;

    IndexedImagePixelOrder pixelOrder =
        IndexedImagePixelOrder::RowMajor;
};

struct IndexedImageColor {
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;
};

struct IndexedImagePixel {
    std::size_t sourceOffset = 0;
    std::uint8_t paletteIndex = 0;
};

struct IndexedImageFile {
    IndexedImageMetadata metadata;
    std::vector<IndexedImageColor> palette;
    std::vector<IndexedImagePixel> pixels;
};

}
