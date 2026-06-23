#pragma once

#include "texture/TextureAsset.h"

namespace eld::render {

class TextureSampler {
public:
    const eld::texture::RgbaPixel* sample(
        const eld::texture::TextureAsset& texture,
        float u,
        float v
    ) const;
};

}
