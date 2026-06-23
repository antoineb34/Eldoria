#include "TextureSampler.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace eld::render {

const eld::texture::RgbaPixel*
TextureSampler::sample(
    const eld::texture::TextureAsset& texture,
    float u,
    float v
) const {
    const std::size_t width =
        texture.width;

    const std::size_t height =
        texture.height;

    if (
        width == 0 ||
        height == 0 ||
        texture.pixels.empty()
    ) {
        return nullptr;
    }

    u = std::clamp(
        u,
        0.0f,
        1.0f
    );

    v =
        v -
        std::floor(v);

    const std::size_t x =
        static_cast<std::size_t>(
            u *
            static_cast<float>(
                width - 1
            )
        );

    const std::size_t y =
        static_cast<std::size_t>(
            v *
            static_cast<float>(
                height - 1
            )
        );

    const std::size_t pixelIndex =
        y * width + x;

    if (
        pixelIndex >=
        texture.pixels.size()
    ) {
        return nullptr;
    }

    return &texture.pixels[
        pixelIndex
    ];
}

}
