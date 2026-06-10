#pragma once

#include <cstdint>
#include <vector>

#include "TextureAsset.h"

namespace eld::texture {

struct TextureFile {
    int id = -1;

    TexturePalette palette;
    TextureMetadata metadata;

    std::vector<uint8_t> indexedPixels;
};

}
