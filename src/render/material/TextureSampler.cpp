#include "TextureSampler.h"

#include <algorithm>
#include <cmath>


namespace eld::render {

const eld::texture::RgbaColor* TextureSampler::sample(
    const eld::texture::TextureAsset& texture,
    float u,
    float v
) const {
    const int width =
        texture.metadata.canvasWidth;

    const int height =
        texture.metadata.canvasHeight;

    if (
        width <= 0 ||
        height <= 0 ||
        texture.pixels.empty()
    ) {
        return nullptr;
    }

    u = std::clamp(u, 0.0f, 1.0f);
    v = v - std::floor(v);

    const int tx =
        static_cast<int>(
            u * static_cast<float>(width - 1)
        );

    const int ty =
        static_cast<int>(
            v * static_cast<float>(height - 1)
        );

    const int pixelIndex =
        ty * width + tx;

    if (
        pixelIndex < 0 ||
        pixelIndex >= static_cast<int>(texture.pixels.size())
    ) {
        return nullptr;
    }

    return &texture.pixels[pixelIndex];
}

}
