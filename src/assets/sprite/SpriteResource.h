#pragma once

#include <cstdint>
#include <string>

#include "image/ImageData.h"

namespace eld::sprite {

struct SpriteResource {
    std::string groupName;
    std::uint16_t frameId = 0;
    eld::image::ImageData image;
};

}
