#pragma once

#include <cstdint>
#include <span>
#include <string>

#include "Font.h"
#include "FontFile.h"

namespace eld::font {

class FontDecoder {
public:
    Font decode(
        std::span<const std::uint8_t> dataPayload,
        std::span<const std::uint8_t> indexPayload,
        std::string name
    ) const;

private:
    Font decodeFile(
        const FontFile& file,
        std::string name
    ) const;

    Glyph decodeGlyph(
        const FontGlyphFile& file
    ) const;
};

}
