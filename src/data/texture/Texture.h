#pragma once

#include <cstdint>

#include "TextureFile.h"
#include "TextureImage.h"
#include "TextureSourceMap.h"

namespace eld::texture {

struct Texture {
    std::uint16_t id = 0;

    TextureFile file;
    TextureImage image;
    TextureSourceMap sourceMap;
};

}
