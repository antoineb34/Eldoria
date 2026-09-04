#include "views/midi/MidiView.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

#include "Midi.h"
#include "midi/MidiTimeline.h"

#include "views/midi/MidiViewState.h"

namespace eld::elforge {

namespace {

std::vector<float> buildActivity(
    const eld::audio::MidiTimeline& timeline,
    std::size_t binCount = 360
) {
    std::vector<float> activity(
        std::max<std::size_t>(
            binCount,
            1
        ),
        0.0f
    );

    if (
        timeline.noteOnEvents.empty() ||
        timeline.totalTicks == 0
    ) {
        return activity;
    }

    for (
        const eld::audio::MidiNoteEvent& event :
        timeline.noteOnEvents
    ) {
        const std::size_t index =
            std::min(
                static_cast<std::size_t>(
                    (
                        static_cast<std::uint64_t>(
                            event.tick
                        ) *
                        activity.size()
                    ) /
                    (
                        static_cast<std::uint64_t>(
                            timeline.totalTicks
                        ) + 1u
                    )
                ),
                activity.size() - 1
            );

        // ElForge visualization policy. This intentionally
        // lives in the tool rather than the shared audio module.
        const float velocity =
            static_cast<float>(
                event.velocity
            ) /
            127.0f;

        const float pitchWeight =
            0.85f +
            0.15f *
                static_cast<float>(
                    event.note
                ) /
                127.0f;

        activity[index] +=
            velocity *
            pitchWeight;
    }

    const float peak =
        *std::max_element(
            activity.begin(),
            activity.end()
        );

    if (peak > 0.0f) {
        for (float& value : activity) {
            value =
                std::sqrt(
                    value / peak
                );
        }
    }

    return activity;
}

}

void MidiView::update(
    const eld::midi::Midi* midi,
    MidiViewState& state
) const {
    if (midi == nullptr) {
        state.midiId = -1;
        state.totalTicks = 0;
        state.activity.clear();
        return;
    }

    const int midiId =
        static_cast<int>(
            midi->id
        );

    if (state.midiId == midiId) {
        return;
    }

    state.seekTick = 0;
    state.seekActive = false;

    const eld::audio::MidiTimeline timeline =
        eld::audio::readMidiTimeline(
            *midi
        );

    state.activity =
        buildActivity(
            timeline
        );

    state.totalTicks =
        static_cast<int>(
            timeline.totalTicks
        );

    state.midiId =
        midiId;
}

}
