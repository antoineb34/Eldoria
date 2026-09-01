#pragma once

#include <cstdint>
#include <vector>

#include "MapTile.h"

namespace eld::map {

struct MapObjectSpawn {
    std::uint16_t id = 0;
    std::uint8_t plane = 0;
    std::uint8_t x = 0;
    std::uint8_t y = 0;
    std::uint8_t type = 0;
    std::uint8_t rotation = 0;
};

struct MapRegion {
    std::uint16_t regionId = 0;
    std::uint16_t terrainFileId = 0;
    std::uint16_t objectFileId = 0;
    bool shouldPreload = false;

    MapTileArray tiles{};
    std::vector<MapObjectSpawn> objects;

    int regionX() const {
        return static_cast<int>(regionId >> 8);
    }

    int regionY() const {
        return static_cast<int>(regionId & 0xFFu);
    }

    int worldBaseX() const {
        return regionX() * static_cast<int>(RegionSize);
    }

    int worldBaseY() const {
        return regionY() * static_cast<int>(RegionSize);
    }

    const MapTile& tile(
        std::size_t plane,
        std::size_t x,
        std::size_t y
    ) const {
        return tiles.at(tileIndex(plane, x, y));
    }
};

struct MapIndexEntry {
    std::uint16_t regionId = 0;
    std::uint16_t terrainFileId = 0;
    std::uint16_t objectFileId = 0;
    bool shouldPreload = false;

    int regionX() const {
        return static_cast<int>(regionId >> 8);
    }

    int regionY() const {
        return static_cast<int>(regionId & 0xFFu);
    }
};

}
