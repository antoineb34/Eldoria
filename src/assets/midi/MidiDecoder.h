#pragma once

#include <cstdint>
#include <span>

#include "midi/MidiData.h"

namespace eld::midi {

class MidiDecoder {
public:
  MidiData decode(std::span<const std::uint8_t> payload) const;
};

} // namespace eld::midi
