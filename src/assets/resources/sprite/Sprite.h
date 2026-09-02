#pragma once

#include <cstdint>
#include <string>

#include "image/Image.h"
#include "image/IndexedImageFile.h"
#include "image/IndexedImageSourceMap.h"

namespace eld::sprite {

struct Sprite {
    std::string groupName;
    std::uint16_t frameId = 0;

    eld::image::IndexedImageFile file;
    eld::image::Image image;
    eld::image::IndexedImageSourceMap sourceMap;
};

}
