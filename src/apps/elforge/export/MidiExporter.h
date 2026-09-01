#pragma once

#include <filesystem>
#include <string>

#include "midi/MidiFile.h"

namespace eld::elforge {

std::filesystem::path defaultMidiExportPath(
    const eld::midi::MidiFile& file
);

bool exportMidi(
    const eld::midi::MidiFile& file,
    const std::filesystem::path& path,
    std::string& error
);

}
