#pragma once

#include <string>
#include <vector>

namespace eld::elforge {

struct MidiViewState {
    int midiId = -1;
    int totalTicks = 0;
    std::vector<float> activity;

    std::string playbackStatus;

    int seekTick = 0;
    bool seekActive = false;
};

}
