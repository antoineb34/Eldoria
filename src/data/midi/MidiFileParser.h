#pragma once

#include <cstdint>
#include <optional>
#include <span>

#include "MidiFile.h"

namespace eld::midi {

class MidiFileParser {
public:
    std::optional<MidiFileData> parse(
        std::span<const std::uint8_t> bytes
    ) const;
};

}
