#pragma once

#include <cstdint>
#include <vector>

namespace eld::font {

enum class FontPixelOrder : std::uint8_t {
    RowMajor = 0,
    ColumnMajor = 1
};

struct FontGlyphFile {
    std::uint16_t character = 0;

    std::uint8_t offsetX = 0;
    std::uint8_t offsetY = 0;

    std::uint16_t width = 0;
    std::uint16_t height = 0;

    FontPixelOrder pixelOrder =
        FontPixelOrder::RowMajor;

    std::vector<std::uint8_t> pixels;
};

struct FontFile {
    std::vector<std::uint8_t> dataPayload;
    std::vector<std::uint8_t> indexPayload;
    std::vector<FontGlyphFile> glyphs;
};

}
