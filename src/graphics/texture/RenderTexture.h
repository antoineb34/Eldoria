#pragma once

#include <cstdint>
#include <vector>

namespace eld::graphics {

enum class PixelFormat : std::uint8_t {
    Rgba8
};

struct RenderTexture {
    std::uint32_t width = 0;
    std::uint32_t height = 0;

    PixelFormat format =
        PixelFormat::Rgba8;

    std::vector<std::uint8_t> pixels;
};

}
