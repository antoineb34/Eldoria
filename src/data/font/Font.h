#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "FontFile.h"

namespace eld::font {

struct Glyph {
    std::uint16_t character = 0;

    std::uint8_t offsetX = 0;
    std::uint8_t offsetY = 0;

    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::uint16_t advance = 0;

    std::vector<std::uint8_t> alpha;
};

struct Font {
    std::string name;
    std::uint16_t lineHeight = 0;

    FontFile file;
    std::vector<Glyph> glyphs;
};

}
