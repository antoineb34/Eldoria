#pragma once

#include "RenderTexture.h"
#include "texture/TextureAsset.h"

namespace eld::graphics {

class RenderTextureBuilder {
public:
    RenderTexture build(
        const eld::texture::TextureAsset& source
    ) const;
};

}
