#pragma once

#include "Texture.h"

#include <vector>
#include <cstdint>

namespace rf::texture {

class TextureIndexParser {
public:
    static TextureIndex parse(
        const std::vector<std::uint8_t>& data
    );
};

}
