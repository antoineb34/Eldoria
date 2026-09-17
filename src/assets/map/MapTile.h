#pragma once

#include <cstddef>
#include <cstdint>

namespace eld::map {

inline constexpr std::size_t PlaneCount = 4;
inline constexpr std::size_t RegionSize = 64;
inline constexpr std::size_t RegionTileCount =
    PlaneCount * RegionSize * RegionSize;

enum class TileFlag : std::uint8_t {
    Solid = 0x01,
    Bridge = 0x02,
    Roof = 0x04,
    ForceLevelZero = 0x08,
    LowMemoryHidden = 0x10,
    Unknown20 = 0x20
};

constexpr std::uint8_t tileFlagMask(TileFlag flag) {
    return static_cast<std::uint8_t>(flag);
}

constexpr bool hasTileFlag(
    std::uint8_t settings,
    TileFlag flag
) {
    return (settings & tileFlagMask(flag)) != 0;
}

struct MapTile {
    int height = 0;

    // Raw cache IDs. Zero means no floor; non-zero IDs map to
    // floor definition (raw ID - 1).
    std::uint8_t overlayId = 0;
    std::uint8_t underlayId = 0;
    std::uint8_t settings = 0;
    std::uint8_t overlayShape = 0;
    std::uint8_t overlayRotation = 0;

    constexpr bool hasFlag(TileFlag flag) const {
        return hasTileFlag(settings, flag);
    }
};

constexpr std::size_t tileIndex(
    std::size_t plane,
    std::size_t x,
    std::size_t y
) {
    return plane * RegionSize * RegionSize
         + x * RegionSize
         + y;
}

}