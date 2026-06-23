#pragma once

#include "../../material/SamplerState.h"

#include "texture/TextureAsset.h"

namespace eld::render {

class TextureSampler {
public:
    eld::texture::RgbaPixel sample(
        const eld::texture::TextureAsset& texture,
        float u,
        float v,
        const SamplerState& state
    ) const;

private:
    float address(
        float coordinate,
        TextureAddressMode mode
    ) const;

    eld::texture::RgbaPixel sampleNearest(
        const eld::texture::TextureAsset& texture,
        float u,
        float v
    ) const;

    eld::texture::RgbaPixel sampleLinear(
        const eld::texture::TextureAsset& texture,
        float u,
        float v,
        const SamplerState& state
    ) const;
};

}
