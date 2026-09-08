#pragma once

#include <cstdint>
#include <span>

#include "sequence/SequenceData.h"

namespace eld::sequence {

class SequenceDecoder {
public:
    SequenceData decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
