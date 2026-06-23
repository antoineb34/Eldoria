#pragma once

#include <cstddef>
#include <vector>

#include "GraphicsTexture.h"
#include "TextureHandle.h"

namespace eld::graphics {

class TextureRegistry {
public:
    TextureHandle registerTexture(
        GraphicsTexture texture
    );

    const GraphicsTexture& get(
        TextureHandle handle
    ) const;

    bool contains(
        TextureHandle handle
    ) const;

    std::size_t count() const;

private:
    std::vector<GraphicsTexture> textures_;
};

}
