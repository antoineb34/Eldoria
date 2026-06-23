#pragma once

#include "SamplerState.h"

#include "texture/TextureAsset.h"

#include "../math/Vec4.h"

namespace eld::render {

enum class BlendMode {
    Opaque,
    Alpha
};

struct Material {
    Vec4 baseColor{
        1.0f,
        1.0f,
        1.0f,
        1.0f
    };

    const eld::texture::TextureAsset*
        albedoTexture = nullptr;

    SamplerState sampler;
    BlendMode blendMode =
        BlendMode::Opaque;

    bool textured() const {
        return albedoTexture != nullptr;
    }

    bool transparent() const {
        return
            blendMode ==
            BlendMode::Alpha;
    }
};

}
