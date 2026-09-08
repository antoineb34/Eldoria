#pragma once

#include <cstdint>
#include <span>

#include "Varbit.h"

namespace eld::varbit {

class VarbitDecoder {
public:
    Varbit decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
