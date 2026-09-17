#include "SoftwareTextureSampler.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace eld::render {

namespace {

ColorPixel readPixel(
    const TextureResource& texture,
    std::size_t x,
    std::size_t y
) {
    const std::size_t index =
        (
            y *
            static_cast<std::size_t>(texture.width) +
            x
        ) * 4u;

    if (index + 3u >= texture.rgba.size()) {
        return {};
    }

    return {
        texture.rgba[index],
        texture.rgba[index + 1u],
        texture.rgba[index + 2u],
        texture.rgba[index + 3u]
    };
}

std::uint8_t interpolate(
    std::uint8_t topLeft,
    std::uint8_t topRight,
    std::uint8_t bottomLeft,
    std::uint8_t bottomRight,
    float horizontal,
    float vertical
) {
    const float top =
        static_cast<float>(topLeft) +
        (
            static_cast<float>(topRight) -
            static_cast<float>(topLeft)
        ) * horizontal;

    const float bottom =
        static_cast<float>(bottomLeft) +
        (
            static_cast<float>(bottomRight) -
            static_cast<float>(bottomLeft)
        ) * horizontal;

    const float value =
        top + (bottom - top) * vertical;

    return static_cast<std::uint8_t>(
        std::clamp(value, 0.0f, 255.0f)
    );
}

}

float SoftwareTextureSampler::address(
    float coordinate,
    TextureAddressMode mode
) const {
    if (mode == TextureAddressMode::Clamp) {
        return std::clamp(
            coordinate,
            0.0f,
            1.0f
        );
    }

    return coordinate - std::floor(coordinate);
}

ColorPixel SoftwareTextureSampler::sample(
    const TextureResource& texture,
    float u,
    float v,
    const TextureSampler& state
) const {
    const std::size_t expected =
        static_cast<std::size_t>(texture.width) *
        static_cast<std::size_t>(texture.height) *
        4u;

    if (
        texture.width == 0 ||
        texture.height == 0 ||
        texture.rgba.size() < expected
    ) {
        return {};
    }

    u = address(u, state.addressU);
    v = address(v, state.addressV);

    if (state.filter == TextureFilter::Linear) {
        return sampleLinear(texture, u, v);
    }

    return sampleNearest(texture, u, v);
}

ColorPixel SoftwareTextureSampler::sampleNearest(
    const TextureResource& texture,
    float u,
    float v
) const {
    const std::size_t width = texture.width;
    const std::size_t height = texture.height;

    const std::size_t x =
        std::min(
            static_cast<std::size_t>(
                u * static_cast<float>(width)
            ),
            width - 1u
        );

    const std::size_t y =
        std::min(
            static_cast<std::size_t>(
                v * static_cast<float>(height)
            ),
            height - 1u
        );

    return readPixel(texture, x, y);
}

ColorPixel SoftwareTextureSampler::sampleLinear(
    const TextureResource& texture,
    float u,
    float v
) const {
    const std::size_t width = texture.width;
    const std::size_t height = texture.height;

    const float x =
        u * static_cast<float>(width - 1u);

    const float y =
        v * static_cast<float>(height - 1u);

    const std::size_t x0 =
        static_cast<std::size_t>(std::floor(x));

    const std::size_t y0 =
        static_cast<std::size_t>(std::floor(y));

    const std::size_t x1 =
        std::min(x0 + 1u, width - 1u);

    const std::size_t y1 =
        std::min(y0 + 1u, height - 1u);

    const float horizontal =
        x - static_cast<float>(x0);

    const float vertical =
        y - static_cast<float>(y0);

    const ColorPixel topLeft =
        readPixel(texture, x0, y0);

    const ColorPixel topRight =
        readPixel(texture, x1, y0);

    const ColorPixel bottomLeft =
        readPixel(texture, x0, y1);

    const ColorPixel bottomRight =
        readPixel(texture, x1, y1);

    return {
        interpolate(
            topLeft.red,
            topRight.red,
            bottomLeft.red,
            bottomRight.red,
            horizontal,
            vertical
        ),
        interpolate(
            topLeft.green,
            topRight.green,
            bottomLeft.green,
            bottomRight.green,
            horizontal,
            vertical
        ),
        interpolate(
            topLeft.blue,
            topRight.blue,
            bottomLeft.blue,
            bottomRight.blue,
            horizontal,
            vertical
        ),
        interpolate(
            topLeft.alpha,
            topRight.alpha,
            bottomLeft.alpha,
            bottomRight.alpha,
            horizontal,
            vertical
        )
    };
}

}
