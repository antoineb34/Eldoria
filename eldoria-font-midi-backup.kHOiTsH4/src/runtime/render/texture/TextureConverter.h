#pragma once

#include "GraphicsTexture.h"
#include "image/ImageData.h"

namespace eld::render {

class TextureConverter {
public:
    GraphicsTexture convert(
        const eld::image::ImageData& source
    ) const;
};

}
