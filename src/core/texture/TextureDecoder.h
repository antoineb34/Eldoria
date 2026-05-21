#pragma once

#include "Texture.h"

#include <vector>
#include <cstdint>

namespace rf::texture {

class TextureDecoder {
public:
    static DecodedTexture decode(
        const TextureIndex& index,
        const std::vector<std::uint8_t>& fileData,
        int textureId = 0
    );
};

}
