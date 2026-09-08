#pragma once

#include <cstdint>
#include <vector>

namespace eld::midi {
struct Midi;
}

namespace eld::audio {

struct MidiNoteEvent {
    std::uint32_t tick = 0;
    std::uint8_t channel = 0;
    std::uint8_t note = 0;
    std::uint8_t velocity = 0;
};

struct MidiTimeline {
    std::vector<MidiNoteEvent> noteOnEvents;
    std::uint32_t totalTicks = 0;
};

MidiTimeline readMidiTimeline(
    const eld::midi::Midi& midi
);

}
