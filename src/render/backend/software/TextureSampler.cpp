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
    eld::graphics::TextureAddressMode mode
) {
    if (mode == eld::graphics::TextureAddressMode::Clamp) {
        return std::clamp(index, 0, size - 1);
    }

    const int remainder = index % size;

    return remainder < 0
        ? remainder + size
        : remainder;
}

ColorPixel readPixel(
    const eld::graphics::GraphicsTexture& texture,
    std::size_t x,
    std::size_t y
) {
    const std::size_t index =
        (
            y * texture.width +
            x
        ) * 4;

    return {
        texture.pixels.at(index),
        texture.pixels.at(index + 1),
        texture.pixels.at(index + 2),
        texture.pixels.at(index + 3)
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
    eld::graphics::TextureAddressMode mode
) const {
    if (mode == eld::graphics::TextureAddressMode::Clamp) {
        return std::clamp(
            coordinate,
            0.0f,
            1.0f
        );
    }

    return coordinate - std::floor(coordinate);
}

ColorPixel TextureSampler::sample(
    const eld::graphics::GraphicsTexture& texture,
    float u,
    float v,
    const eld::graphics::SamplerState& state
) const {
    const std::size_t expectedSize =
        static_cast<std::size_t>(texture.width) *
        static_cast<std::size_t>(texture.height) *
        4;

    if (
        texture.format !=
            eld::graphics::TextureFormat::Rgba8 ||
        texture.width == 0 ||
        texture.height == 0 ||
        texture.pixels.size() < expectedSize
    ) {
        return {};
    }

    u = address(u, state.addressU);
    v = address(v, state.addressV);

    switch (state.filter) {
        case eld::graphics::TextureFilter::Nearest:
            return sampleNearest(
                texture,
                u,
                v
            );

        case eld::graphics::TextureFilter::Linear:
            return sampleLinear(
                texture,
                u,
                v,
                state
            );
    }

    return {};
}

ColorPixel TextureSampler::sampleNearest(
    const eld::graphics::GraphicsTexture& texture,
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
            width - 1
        );

    const std::size_t y =
        std::min(
            static_cast<std::size_t>(
                v * static_cast<float>(height)
            ),
            height - 1
        );

    return readPixel(
        texture,
        x,
        y
    );
}

ColorPixel TextureSampler::sampleLinear(
    const eld::graphics::GraphicsTexture& texture,
    float u,
    float v,
    const eld::graphics::SamplerState& state
) const {
    const int width =
        static_cast<int>(texture.width);

    const int height =
        static_cast<int>(texture.height);

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
        sourceX - std::floor(sourceX);

    const float vertical =
        sourceY - std::floor(sourceY);

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

    const ColorPixel topLeft =
        readPixel(
            texture,
            addressedX0,
            addressedY0
        );

    const ColorPixel topRight =
        readPixel(
            texture,
            addressedX1,
            addressedY0
        );

    const ColorPixel bottomLeft =
        readPixel(
            texture,
            addressedX0,
            addressedY1
        );

    const ColorPixel bottomRight =
        readPixel(
            texture,
            addressedX1,
            addressedY1
        );

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
