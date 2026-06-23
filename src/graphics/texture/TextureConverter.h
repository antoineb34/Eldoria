#pragma once

#include "GraphicsTexture.h"
#include "texture/TextureImage.h"

namespace eld::graphics {

class TextureConverter {
public:
    GraphicsTexture convert(
        const eld::texture::TextureImage& source
    ) const;
};

}
