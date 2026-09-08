#pragma once

#include <cstdint>
#include <vector>

namespace eld::midi {

struct MidiEvent {
    std::uint32_t tick = 0;
    std::vector<std::uint8_t> message;
};


struct MidiTrack {
    std::vector<MidiEvent> events;
};


struct Midi {
    std::uint16_t id = 0;

    std::uint16_t division = 0;
    std::uint32_t totalTicks = 0;

    std::vector<MidiTrack> tracks;
    std::vector<std::uint8_t> bytes;
};

}
