#pragma once

#include <cstdint>

#include "image/ImageData.h"

namespace eld::texture {

struct TextureResource {
    std::uint16_t id = 0;
    eld::image::ImageData image;
};

}
