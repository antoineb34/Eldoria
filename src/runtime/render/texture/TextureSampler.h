#pragma once

#include <cstdint>

namespace eld::render {

enum class TextureFilter : std::uint8_t {
    Nearest,
    Linear
};

enum class TextureAddressMode : std::uint8_t {
    Repeat,
    Clamp
};

struct TextureSampler {
    TextureFilter filter =
        TextureFilter::Nearest;

    TextureAddressMode addressU =
        TextureAddressMode::Repeat;

    TextureAddressMode addressV =
        TextureAddressMode::Repeat;
};

}
