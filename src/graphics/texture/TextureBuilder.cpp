#include "TextureBuilder.h"

namespace eld::graphics
{

eld::render::TextureResource TextureBuilder::build(
    const eld::image::ImageData& image) const
{
    eld::render::TextureResource resource;

    resource.width = image.width;
    resource.height = image.height;

    resource.rgba.reserve(
        image.pixels.size() * 4
    );

    for (const auto& pixel : image.pixels)
    {
        resource.rgba.push_back(pixel.red);
        resource.rgba.push_back(pixel.green);
        resource.rgba.push_back(pixel.blue);
        resource.rgba.push_back(pixel.alpha);
    }

    return resource;
}

}
