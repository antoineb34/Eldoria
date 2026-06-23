#pragma once

#include <cstdint>
#include <vector>

namespace eld::texture {

struct RgbaPixel {
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;
    std::uint8_t alpha = 0;
};

struct TextureImage {
    std::uint16_t width = 0;
    std::uint16_t height = 0;

    std::vector<RgbaPixel> pixels;
};

}
