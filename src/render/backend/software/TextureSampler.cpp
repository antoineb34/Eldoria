#include "TextureSampler.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace eld::render {

namespace {

int addressIndex(
    int index,
    int size,
    TextureAddressMode mode
) {
    if (mode == TextureAddressMode::Clamp) {
        return std::clamp(
            index,
            0,
            size - 1
        );
    }

    const int remainder =
        index % size;

    return
        remainder < 0
            ? remainder + size
            : remainder;
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
        static_cast<float>(topLeft) *
            (1.0f - horizontal) +
        static_cast<float>(topRight) *
            horizontal;

    const float bottom =
        static_cast<float>(bottomLeft) *
            (1.0f - horizontal) +
        static_cast<float>(bottomRight) *
            horizontal;

    return static_cast<std::uint8_t>(
        std::clamp(
            top * (1.0f - vertical) +
                bottom * vertical,
            0.0f,
            255.0f
        )
    );
}

}

float TextureSampler::address(
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

    return
        coordinate -
        std::floor(coordinate);
}

eld::texture::RgbaPixel
TextureSampler::sample(
    const eld::texture::TextureAsset& texture,
    float u,
    float v,
    const SamplerState& state
) const {
    const std::size_t expectedPixelCount =
        static_cast<std::size_t>(
            texture.width
        ) *
        static_cast<std::size_t>(
            texture.height
        );

    if (
        texture.width == 0 ||
        texture.height == 0 ||
        texture.pixels.size() <
            expectedPixelCount
    ) {
        return {};
    }

    u = address(
        u,
        state.addressU
    );

    v = address(
        v,
        state.addressV
    );

    switch (state.filter) {
        case TextureFilter::Nearest:
            return sampleNearest(
                texture,
                u,
                v
            );

        case TextureFilter::Linear:
            return sampleLinear(
                texture,
                u,
                v,
                state
            );
    }

    return {};
}

eld::texture::RgbaPixel
TextureSampler::sampleNearest(
    const eld::texture::TextureAsset& texture,
    float u,
    float v
) const {
    const std::size_t width =
        texture.width;

    const std::size_t height =
        texture.height;

    const std::size_t x =
        std::min(
            static_cast<std::size_t>(
                u *
                static_cast<float>(width)
            ),
            width - 1
        );

    const std::size_t y =
        std::min(
            static_cast<std::size_t>(
                v *
                static_cast<float>(height)
            ),
            height - 1
        );

    return texture.pixels[
        y * width + x
    ];
}

eld::texture::RgbaPixel
TextureSampler::sampleLinear(
    const eld::texture::TextureAsset& texture,
    float u,
    float v,
    const SamplerState& state
) const {
    const int width =
        texture.width;

    const int height =
        texture.height;

    const float sourceX =
        u * static_cast<float>(width) -
        0.5f;

    const float sourceY =
        v * static_cast<float>(height) -
        0.5f;

    const int x0 =
        static_cast<int>(
            std::floor(sourceX)
        );

    const int y0 =
        static_cast<int>(
            std::floor(sourceY)
        );

    const int x1 = x0 + 1;
    const int y1 = y0 + 1;

    const float horizontal =
        sourceX -
        std::floor(sourceX);

    const float vertical =
        sourceY -
        std::floor(sourceY);

    const int addressedX0 =
        addressIndex(
            x0,
            width,
            state.addressU
        );

    const int addressedX1 =
        addressIndex(
            x1,
            width,
            state.addressU
        );

    const int addressedY0 =
        addressIndex(
            y0,
            height,
            state.addressV
        );

    const int addressedY1 =
        addressIndex(
            y1,
            height,
            state.addressV
        );

    const auto& topLeft =
        texture.pixels[
            addressedY0 * width +
            addressedX0
        ];

    const auto& topRight =
        texture.pixels[
            addressedY0 * width +
            addressedX1
        ];

    const auto& bottomLeft =
        texture.pixels[
            addressedY1 * width +
            addressedX0
        ];

    const auto& bottomRight =
        texture.pixels[
            addressedY1 * width +
            addressedX1
        ];

    return eld::texture::RgbaPixel{
        .red = interpolate(
            topLeft.red,
            topRight.red,
            bottomLeft.red,
            bottomRight.red,
            horizontal,
            vertical
        ),
        .green = interpolate(
            topLeft.green,
            topRight.green,
            bottomLeft.green,
            bottomRight.green,
            horizontal,
            vertical
        ),
        .blue = interpolate(
            topLeft.blue,
            topRight.blue,
            bottomLeft.blue,
            bottomRight.blue,
            horizontal,
            vertical
        ),
        .alpha = interpolate(
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
