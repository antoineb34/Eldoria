#pragma once

#include "GraphicsTexture.h"
#include "image/Image.h"

namespace eld::render {

class TextureConverter {
public:
    GraphicsTexture convert(
        const eld::image::Image& source
    ) const;
};

}
