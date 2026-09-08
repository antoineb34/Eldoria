#pragma once

#include "ColorBuffer.h"
#include "render/texture/GraphicsTexture.h"
#include "render/texture/SamplerState.h"

namespace eld::render {

class TextureSampler {
public:
    ColorPixel sample(
        const eld::render::GraphicsTexture& texture,
        float u,
        float v,
        const eld::render::SamplerState& state
    ) const;

private:
    float address(
        float coordinate,
        eld::render::TextureAddressMode mode
    ) const;

    ColorPixel sampleNearest(
        const eld::render::GraphicsTexture& texture,
        float u,
        float v
    ) const;

    ColorPixel sampleLinear(
        const eld::render::GraphicsTexture& texture,
        float u,
        float v,
        const eld::render::SamplerState& state
    ) const;
};

}
