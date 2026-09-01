#pragma once

#include <imgui.h>

namespace eld::audio {
class MidiPlayer;
}

namespace eld::midi {
struct MidiFile;
}

namespace eld::elforge {

struct MidiViewState;

class MidiViewPanel {
public:
    void renderVisualization(
        MidiViewState& viewState,
        eld::audio::MidiPlayer& midiPlayer,
        const ImVec2& size
    );

    void renderControls(
        const eld::midi::MidiFile* midi,
        MidiViewState& viewState,
        eld::audio::MidiPlayer& midiPlayer
    );
};

}
