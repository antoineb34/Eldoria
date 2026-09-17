#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include "texture/TextureLoader.h"

#include "render/texture/TextureHandle.h"
#include "render/texture/TextureManager.h"

#include "TextureBuilder.h"

namespace eld::graphics
{

class TextureSystem
{
public:
    TextureSystem(
        const eld::texture::TextureLoader& loader,
        eld::render::TextureManager& manager);

    eld::render::TextureHandle get(
        std::uint16_t textureId);

    std::optional<eld::render::TextureHandle> find(
        std::uint16_t textureId);

private:
    const eld::texture::TextureLoader& loader_;
    eld::render::TextureManager& manager_;

    TextureBuilder builder_;

    std::unordered_map<
        std::uint16_t,
        eld::render::TextureHandle
    > handles_;
};

}
