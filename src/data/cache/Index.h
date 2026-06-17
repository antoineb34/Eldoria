#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>

namespace eld::cache {

enum class IndexId : std::uint8_t {
    Config = 0,
    Models = 1,
    Animations = 2,
    Midi = 3,
    Maps = 4
};

struct IndexEntry {
    static constexpr std::size_t SizeFieldSize = 3;
    static constexpr std::size_t FirstSectorFieldSize = 3;
    static constexpr std::size_t TotalSize = SizeFieldSize + FirstSectorFieldSize;

    std::uint32_t size;
    std::uint32_t firstSector;
};

struct Index {
    IndexId id;
    std::filesystem::path path;
};

}
