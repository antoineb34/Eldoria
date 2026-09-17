#pragma once

#include <cstdint>
#include <vector>

namespace eld::render
{

struct TextureResource
{
    std::uint32_t width = 0;
    std::uint32_t height = 0;

    // RGBA8, four bytes per pixel.
    std::vector<std::uint8_t> rgba;
};

}
