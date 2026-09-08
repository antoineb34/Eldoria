#pragma once

#include <cstdint>

#include "image/ImageData.h"
#include "texture/TextureResource.h"

namespace eld::texture {

class TextureAssembler {
public:
    TextureResource assemble(
        std::uint16_t id,
        eld::image::ImageData data
    ) const;
};

}
