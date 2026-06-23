#pragma once

namespace eld::render {

enum class TextureAddressMode {
    Clamp,
    Repeat
};

enum class TextureFilter {
    Nearest,
    Linear
};

struct SamplerState {
    TextureAddressMode addressU =
        TextureAddressMode::Repeat;

    TextureAddressMode addressV =
        TextureAddressMode::Repeat;

    TextureFilter filter =
        TextureFilter::Nearest;
};

}
