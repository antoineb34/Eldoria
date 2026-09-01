#pragma once

namespace eld::midi {
struct MidiFile;
}

namespace eld::elforge {

struct MidiViewState;

class MidiView {
public:
    void update(
        const eld::midi::MidiFile* midi,
        MidiViewState& state
    ) const;
};

}
