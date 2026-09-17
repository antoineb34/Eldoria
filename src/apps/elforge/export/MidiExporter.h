#pragma once

#include <filesystem>
#include <string>

#include "midi/MidiData.h"

namespace eld::elforge {

std::filesystem::path defaultMidiExportPath(const eld::midi::MidiData &file);

bool exportMidi(const eld::midi::MidiData &file,
                const std::filesystem::path &path, std::string &error);

} // namespace eld::elforge
