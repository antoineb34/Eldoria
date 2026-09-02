#include "TextureRegistry.h"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

namespace eld::render {

TextureHandle TextureRegistry::registerTexture(
    GraphicsTexture texture
) {
    if (
        textures_.size() >
        std::numeric_limits<std::uint32_t>::max()
    ) {
        throw std::overflow_error(
            "Texture registry capacity exceeded"
        );
    }

    const TextureHandle handle{
        static_cast<std::uint32_t>(
            textures_.size()
        )
    };

    textures_.push_back(
        std::move(texture)
    );

    return handle;
}

const GraphicsTexture& TextureRegistry::get(
    TextureHandle handle
) const {
    return textures_.at(
        static_cast<std::size_t>(
            handle.value
        )
    );
}

bool TextureRegistry::contains(
    TextureHandle handle
) const {
    return
        static_cast<std::size_t>(
            handle.value
        ) <
        textures_.size();
}

std::size_t TextureRegistry::count() const {
    return textures_.size();
}

}
