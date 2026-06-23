#include "TextureResolver.h"

#include <utility>

namespace eld::graphics {

TextureResolver::TextureResolver(
    eld::texture::TextureRepository& repository,
    TextureRegistry& registry
)
    : repository_(repository),
      registry_(registry) {
}

TextureHandle TextureResolver::resolve(
    std::uint16_t sourceTextureId
) {
    const auto existing =
        handles_.find(
            sourceTextureId
        );

    if (
        existing !=
        handles_.end()
    ) {
        return existing->second;
    }

    eld::image::Image source =
        repository_.getImage(
            sourceTextureId
        );

    GraphicsTexture texture =
        converter_.convert(
            source
        );

    const TextureHandle handle =
        registry_.registerTexture(
            std::move(texture)
        );

    handles_.emplace(
        sourceTextureId,
        handle
    );

    return handle;
}

}
