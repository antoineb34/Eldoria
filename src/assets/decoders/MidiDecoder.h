#pragma once

#include <cstdint>
#include <span>

#include "Midi.h"

namespace eld::midi {

class MidiDecoder {
public:
    Midi decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
