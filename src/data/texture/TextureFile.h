#pragma once

#include <cstdint>
#include <vector>

#include "TextureAsset.h"

namespace rf::texture {

struct TextureFile {
    int id = -1;

    TexturePalette palette;
    TextureMetadata metadata;

    std::vector<uint8_t> indexedPixels;
};

}
