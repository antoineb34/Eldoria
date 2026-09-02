#pragma once

#include <cstdint>

namespace eld::render {

struct ModelHandle {
    std::uint32_t value = 0;

    bool operator==(
        const ModelHandle&
    ) const = default;
};

}
