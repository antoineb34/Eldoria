#include "TextureConverter.h"

#include <cstddef>
#include <stdexcept>

namespace eld::graphics {

GraphicsTexture TextureConverter::convert(
    const eld::image::Image& source
) const {
    if (
        source.width == 0 ||
        source.height == 0
    ) {
        throw std::invalid_argument(
            "Texture dimensions must be positive"
        );
    }

    const std::size_t expectedPixelCount =
        static_cast<std::size_t>(
            source.width
        ) *
        static_cast<std::size_t>(
            source.height
        );

    if (
        source.pixels.size() !=
        expectedPixelCount
    ) {
        throw std::invalid_argument(
            "Texture pixel count does not match its dimensions"
        );
    }

    GraphicsTexture texture{};

    texture.width =
        source.width;

    texture.height =
        source.height;

    texture.format =
        TextureFormat::Rgba8;

    texture.pixels.reserve(
        expectedPixelCount * 4
    );

    for (
        const eld::image::RgbaPixel& pixel :
        source.pixels
    ) {
        texture.pixels.push_back(
            pixel.red
        );

        texture.pixels.push_back(
            pixel.green
        );

        texture.pixels.push_back(
            pixel.blue
        );

        texture.pixels.push_back(
            pixel.alpha
        );
    }

    return texture;
}

}
