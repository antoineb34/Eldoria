#pragma once

#include <filesystem>
#include <string>

#include "Midi.h"

namespace eld::elforge {

std::filesystem::path defaultMidiExportPath(
    const eld::midi::Midi& file
);

bool exportMidi(
    const eld::midi::Midi& file,
    const std::filesystem::path& path,
    std::string& error
);

}
