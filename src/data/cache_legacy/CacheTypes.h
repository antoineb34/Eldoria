#pragma once

#include <cstdint>
#include <vector>

namespace eld::cache_legacy {

enum class CacheIndex {
    Config = 1,
    Model = 2,
    Animation = 3,
    Midi = 4,
    Map = 5
};

struct CacheIndexEntry {
    std::uint32_t size = 0;
    std::uint32_t firstSector = 0;
};

struct CacheSectorHeader {
    std::uint16_t fileId = 0;
    std::uint16_t chunkId = 0;
    std::uint32_t nextSector = 0;
    CacheIndex index = CacheIndex::Config;
};

struct CacheFile {
    int id = 0;
    CacheIndex index = CacheIndex::Config;
    CacheIndexEntry entry {};
    std::vector<std::uint8_t> payload;
};

}
