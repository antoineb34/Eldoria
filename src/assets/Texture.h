#pragma once

#include <cstdint>

#include "Image.h"

namespace eld::texture {

struct Texture {
    std::uint16_t id = 0;
    eld::image::Image image;
};

}
