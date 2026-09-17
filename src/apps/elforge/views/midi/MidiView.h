#pragma once

namespace eld::midi {
struct MidiData;
}

namespace eld::elforge {

struct MidiViewState;

class MidiView {
public:
  void update(const eld::midi::MidiData *midi, MidiViewState &state) const;
};

} // namespace eld::elforge
