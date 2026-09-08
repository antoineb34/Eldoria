#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "Index.h"

namespace eld::cache {

struct SectorHeader {
    std::uint16_t fileId;
    std::uint16_t chunkId;
    std::uint32_t nextSector;
    IndexId indexId;
};

struct Sector {
    static constexpr std::size_t HeaderSize = 8;
    static constexpr std::size_t DataSize = 512;
    static constexpr std::size_t TotalSize = HeaderSize + DataSize;

    SectorHeader header;
    std::array<std::uint8_t, DataSize> data;
};

}
