#pragma once

#include <cstdint>

namespace eld::render
{

struct TextureHandle
{
    std::uint32_t index = 0;
    std::uint32_t generation = 0;

    bool operator==(const TextureHandle&) const = default;
};

}
