#pragma once

#include <cstddef>
#include <vector>

#include "RenderTexture.h"
#include "TextureHandle.h"

namespace eld::graphics {

class TextureStore {
public:
    TextureHandle add(
        RenderTexture texture
    );

    const RenderTexture& get(
        TextureHandle handle
    ) const;

    bool contains(
        TextureHandle handle
    ) const;

    std::size_t count() const;

private:
    std::vector<RenderTexture> textures_;
};

}
