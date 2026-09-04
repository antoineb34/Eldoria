#include "MidiTimeline.h"

#include <algorithm>
#include <cstdint>

#include "Midi.h"

namespace eld::audio {

MidiTimeline readMidiTimeline(
    const eld::midi::Midi& midi
) {
    MidiTimeline timeline;

    timeline.totalTicks =
        midi.totalTicks;

    for (const eld::midi::MidiTrack& track :
         midi.tracks) {
        for (const eld::midi::MidiEvent& event :
             track.events) {
            if (event.message.size() < 3) {
                continue;
            }

            const auto status =
                event.message[0];

            const auto command =
                status & 0xF0u;

            const auto velocity =
                event.message[2];

            if (
                command != 0x90u ||
                velocity == 0
            ) {
                continue;
            }

            timeline.noteOnEvents.push_back({
                .tick = event.tick,
                .channel =
                    static_cast<std::uint8_t>(
                        status & 0x0Fu
                    ),
                .note = event.message[1],
                .velocity = velocity
            });
        }
    }

    std::sort(
        timeline.noteOnEvents.begin(),
        timeline.noteOnEvents.end(),
        [](
            const MidiNoteEvent& left,
            const MidiNoteEvent& right
        ) {
            return left.tick < right.tick;
        }
    );

    return timeline;
}

}
