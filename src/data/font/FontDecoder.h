#pragma once

#include <string>

#include "Font.h"
#include "FontFile.h"

namespace eld::font {

class FontDecoder {
public:
    Font decode(
        FontFile file,
        std::string name
    ) const;

private:
    Glyph decodeGlyph(
        const FontGlyphFile& file
    ) const;
};

}
