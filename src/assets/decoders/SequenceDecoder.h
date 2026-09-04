#pragma once

#include <cstdint>
#include <span>

#include "Sequence.h"

namespace eld::sequence {

class SequenceDecoder {
public:
    Sequence decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
