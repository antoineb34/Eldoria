#pragma once

#include <cstdint>

#include "image/IndexedImageFile.h"
#include "image/Image.h"
#include "image/IndexedImageSourceMap.h"

namespace eld::texture {

struct Texture {
    std::uint16_t id = 0;

    eld::image::IndexedImageFile file;
    eld::image::Image image;
    eld::image::IndexedImageSourceMap sourceMap;
};

}
