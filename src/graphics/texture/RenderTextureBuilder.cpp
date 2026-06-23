#include "RenderTextureBuilder.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace eld::graphics {

RenderTexture RenderTextureBuilder::build(
    const eld::texture::TextureAsset& source
) const {
    if (
        source.metadata.canvasWidth <= 0 ||
        source.metadata.canvasHeight <= 0
    ) {
        throw std::invalid_argument(
            "Texture canvas dimensions must be positive"
        );
    }

    const std::uint32_t width =
        static_cast<std::uint32_t>(
            source.metadata.canvasWidth
        );

    const std::uint32_t height =
        static_cast<std::uint32_t>(
            source.metadata.canvasHeight
        );

    const std::size_t expectedPixelCount =
        static_cast<std::size_t>(width) *
        static_cast<std::size_t>(height);

    if (
        source.pixels.size() !=
        expectedPixelCount
    ) {
        throw std::invalid_argument(
            "Texture pixel count does not match canvas dimensions"
        );
    }

    RenderTexture texture{};

    texture.width =
        width;

    texture.height =
        height;

    texture.format =
        PixelFormat::Rgba8;

    texture.pixels.reserve(
        expectedPixelCount * 4
    );

    for (
        const eld::texture::RgbaColor& pixel :
        source.pixels
    ) {
        texture.pixels.push_back(pixel.r);
        texture.pixels.push_back(pixel.g);
        texture.pixels.push_back(pixel.b);
        texture.pixels.push_back(pixel.a);
    }

    return texture;
}

}
