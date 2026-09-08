#pragma once

#include <cstdint>
#include <vector>

namespace eld::map {

struct MapIndexEntry {
    std::uint16_t regionId = 0;
    std::uint16_t terrainFileId = 0;
    std::uint16_t locationFileId = 0;
    bool shouldPreload = false;

    int regionX() const {
        return static_cast<int>(regionId >> 8);
    }

    int regionY() const {
        return static_cast<int>(regionId & 0xFFu);
    }
};

struct MapIndex {
    std::vector<MapIndexEntry> entries;
};

}
