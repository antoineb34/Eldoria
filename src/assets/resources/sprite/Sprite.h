#pragma once

#include <cstdint>
#include <string>

#include "image/Image.h"

namespace eld::sprite {

struct Sprite {
    std::string groupName;
    std::uint16_t frameId = 0;
    eld::image::Image image;
};

}
