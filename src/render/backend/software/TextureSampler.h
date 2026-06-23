#pragma once

#include "ColorBuffer.h"
#include "graphics/texture/GraphicsTexture.h"
#include "graphics/texture/SamplerState.h"

namespace eld::render {

class TextureSampler {
public:
    ColorPixel sample(
        const eld::graphics::GraphicsTexture& texture,
        float u,
        float v,
        const eld::graphics::SamplerState& state
    ) const;

private:
    float address(
        float coordinate,
        eld::graphics::TextureAddressMode mode
    ) const;

    ColorPixel sampleNearest(
        const eld::graphics::GraphicsTexture& texture,
        float u,
        float v
    ) const;

    ColorPixel sampleLinear(
        const eld::graphics::GraphicsTexture& texture,
        float u,
        float v,
        const eld::graphics::SamplerState& state
    ) const;
};

}
