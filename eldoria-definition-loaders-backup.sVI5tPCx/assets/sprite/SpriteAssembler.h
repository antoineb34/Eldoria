#pragma once

#include <cstdint>
#include <string>

#include "image/ImageData.h"
#include "sprite/SpriteResource.h"

namespace eld::sprite {

class SpriteAssembler {
public:
    SpriteResource assemble(
        std::string groupName,
        std::uint16_t frameId,
        eld::image::ImageData data
    ) const;
};

}
