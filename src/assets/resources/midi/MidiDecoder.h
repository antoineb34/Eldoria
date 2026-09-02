#pragma once

#include <cstdint>
#include <span>

#include "MidiFile.h"
#include "MidiFileParser.h"

namespace eld::midi {

class MidiDecoder {
public:
    MidiFile decode(
        std::span<const std::uint8_t> payload
    ) const;

private:
    MidiFileParser parser_;
};

}
