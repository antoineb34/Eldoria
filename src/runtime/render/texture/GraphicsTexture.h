#pragma once

#include <cstdint>
#include <vector>

namespace eld::render {

enum class TextureFormat : std::uint8_t {
    Rgba8
};

struct GraphicsTexture {
    std::uint32_t width = 0;
    std::uint32_t height = 0;

    TextureFormat format =
        TextureFormat::Rgba8;

    std::vector<std::uint8_t> pixels;
};

}
