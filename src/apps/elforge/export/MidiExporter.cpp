#include "export/MidiExporter.h"

#include <fstream>
#include <system_error>

namespace eld::elforge {

std::filesystem::path defaultMidiExportPath(
    const eld::midi::MidiFile& file
) {
    return
        std::filesystem::path("exports") /
        "midi" /
        ("midi_" + std::to_string(file.id) + ".mid");
}

bool exportMidi(
    const eld::midi::MidiFile& file,
    const std::filesystem::path& path,
    std::string& error
) {
    error.clear();

    std::error_code filesystemError;

    const std::filesystem::path parent =
        path.parent_path();

    if (!parent.empty()) {
        std::filesystem::create_directories(
            parent,
            filesystemError
        );

        if (filesystemError) {
            error =
                "Failed to create export directory: " +
                filesystemError.message();
            return false;
        }
    }

    std::ofstream stream(
        path,
        std::ios::binary | std::ios::trunc
    );

    if (!stream.is_open()) {
        error = "Failed to open export file";
        return false;
    }

    if (!file.data.bytes.empty()) {
        stream.write(
            reinterpret_cast<const char*>(
                file.data.bytes.data()
            ),
            static_cast<std::streamsize>(
                file.data.bytes.size()
            )
        );
    }

    if (!stream) {
        error = "Failed while writing MIDI export";
        return false;
    }

    return true;
}

}
