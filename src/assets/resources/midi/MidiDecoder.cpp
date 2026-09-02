#include "MidiDecoder.h"

#include <optional>
#include <stdexcept>
#include <utility>

namespace eld::midi {

MidiFile MidiDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    std::optional<MidiFileData> data =
        parser_.parse(payload);

    if (!data.has_value()) {
        throw std::runtime_error(
            "Invalid MIDI payload"
        );
    }

    return MidiFile{
        .data = std::move(*data)
    };
}

}
