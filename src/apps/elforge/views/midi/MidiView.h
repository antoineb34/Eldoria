#pragma once

namespace eld::midi {
struct Midi;
}

namespace eld::elforge {

struct MidiViewState;

class MidiView {
public:
    void update(
        const eld::midi::Midi* midi,
        MidiViewState& state
    ) const;
};

}
