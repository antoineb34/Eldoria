#pragma once

#include "image/ImageData.h"
#include "render/texture/TextureResource.h"

namespace eld::graphics
{

class TextureBuilder
{
public:
    eld::render::TextureResource build(
        const eld::image::ImageData& image) const;
};

}
