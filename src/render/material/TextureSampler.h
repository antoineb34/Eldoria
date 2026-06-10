#pragma once

#include "texture/TextureAsset.h"
#include "../backend/software/ColorBuffer.h"

namespace eld::render {

class TextureSampler {
public:
    const eld::texture::RgbaColor* sample(
        const eld::texture::TextureAsset& texture,
        float u,
        float v
    ) const;
};

}
