#pragma once

#include <cstdint>
#include <span>

#include "Varp.h"

namespace eld::varp {

class VarpDecoder {
public:
    Varp decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
