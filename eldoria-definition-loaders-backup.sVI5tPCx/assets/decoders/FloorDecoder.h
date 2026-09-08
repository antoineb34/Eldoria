#pragma once

#include <cstdint>
#include <span>

#include "Floor.h"

namespace eld::floor {

class FloorDecoder {
public:
    Floor decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
