#pragma once

#include <cstdint>
#include <unordered_map>

#include "TextureConverter.h"
#include "TextureHandle.h"
#include "TextureRegistry.h"
#include "texture/TexturePipeline.h"

namespace eld::render {

class TextureResolver {
public:
    TextureResolver(
        eld::texture::TexturePipeline& repository,
        TextureRegistry& registry
    );

    TextureHandle resolve(
        std::uint16_t sourceTextureId
    );

private:
    eld::texture::TexturePipeline& repository_;
    TextureRegistry& registry_;

    TextureConverter converter_;

    std::unordered_map<
        std::uint16_t,
        TextureHandle
    > handles_;
};

}
