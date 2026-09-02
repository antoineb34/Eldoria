#pragma once

#include <cstdint>

namespace eld::render {

struct TextureHandle {
    std::uint32_t value = 0;

    bool operator==(
        const TextureHandle&
    ) const = default;
};

}
