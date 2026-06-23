#pragma once

#include <cstdint>

namespace eld::graphics {

struct TextureHandle {
    std::uint32_t value = 0;

    bool operator==(
        const TextureHandle&
    ) const = default;
};

}
