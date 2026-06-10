#pragma once

#include "texture/TextureAsset.h"
#include "../backend/software/ColorBuffer.h"

namespace rf::render_next {

class TextureSampler {
public:
    const rf::texture::RgbaColor* sample(
        const rf::texture::TextureAsset& texture,
        float u,
        float v
    ) const;
};

}
