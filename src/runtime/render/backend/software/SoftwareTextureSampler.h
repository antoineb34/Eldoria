#pragma once

#include "ColorBuffer.h"
#include "render/texture/TextureResource.h"
#include "render/texture/TextureSampler.h"

namespace eld::render {

class SoftwareTextureSampler {
public:
    ColorPixel sample(
        const TextureResource& texture,
        float u,
        float v,
        const TextureSampler& state
    ) const;

private:
    float address(
        float coordinate,
        TextureAddressMode mode
    ) const;

    ColorPixel sampleNearest(
        const TextureResource& texture,
        float u,
        float v
    ) const;

    ColorPixel sampleLinear(
        const TextureResource& texture,
        float u,
        float v
    ) const;
};

}
