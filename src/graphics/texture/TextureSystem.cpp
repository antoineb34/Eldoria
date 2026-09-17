#include "TextureSystem.h"

#include <utility>

namespace eld::graphics
{

TextureSystem::TextureSystem(
    const eld::texture::TextureLoader& loader,
    eld::render::TextureManager& manager)
    : loader_(loader),
      manager_(manager)
{
}

eld::render::TextureHandle TextureSystem::get(
    std::uint16_t textureId)
{
    const auto existing =
        handles_.find(textureId);

    if (existing != handles_.end())
    {
        if (manager_.isValid(
                existing->second))
        {
            return existing->second;
        }

        handles_.erase(existing);
    }

    const eld::image::ImageData& image =
        loader_.get(textureId);

    eld::render::TextureResource resource =
        builder_.build(image);

    eld::render::TextureHandle handle =
        manager_.create(
            std::move(resource));

    handles_.emplace(
        textureId,
        handle);

    return handle;
}


std::optional<eld::render::TextureHandle>
TextureSystem::find(
    std::uint16_t textureId)
{
    if (!loader_.contains(textureId))
    {
        return std::nullopt;
    }

    return get(textureId);
}

}
