#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace eld::midi {

enum class MidiContainer : std::uint8_t {
    StandardMidi,
    RiffMidi
};

struct MidiHeader {
    std::uint16_t format = 0;
    std::uint16_t trackCount = 0;
    std::uint16_t division = 0;
};

struct MidiTrackInfo {
    std::size_t chunkOffset = 0;
    std::size_t dataOffset = 0;
    std::uint32_t dataSize = 0;
};

struct MidiFileData {
    MidiContainer sourceContainer = MidiContainer::StandardMidi;
    MidiHeader header;
    std::vector<MidiTrackInfo> tracks;

    // Normalized Standard MIDI File bytes. These always begin with MThd,
    // even when the cache source used a RIFF/RMID wrapper.
    std::vector<std::uint8_t> bytes;
};

struct MidiFile {
    std::uint16_t id = 0;
    MidiFileData data;
};

}
