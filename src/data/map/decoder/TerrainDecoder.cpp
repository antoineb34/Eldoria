#include "TerrainDecoder.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>

#include "binary/ByteReader.h"

namespace eld::map {
namespace {

int classicNoise(
    int x,
    int y
) {
    std::int32_t n =
        static_cast<std::int32_t>(x + y * 57);
    n = static_cast<std::int32_t>(
        (static_cast<std::uint32_t>(n) << 13) ^
        static_cast<std::uint32_t>(n)
    );

    const std::uint32_t un =
        static_cast<std::uint32_t>(n);
    const std::uint32_t hashed =
        (
            un *
            (
                un * un * 15731u +
                789221u
            ) +
            1376312589u
        ) &
        0x7fffffffu;

    return static_cast<int>((hashed >> 19) & 0xFFu);
}

int classicSmoothNoise(
    int x,
    int y
) {
    const int corners =
        classicNoise(x - 1, y - 1) +
        classicNoise(x + 1, y - 1) +
        classicNoise(x - 1, y + 1) +
        classicNoise(x + 1, y + 1);
    const int sides =
        classicNoise(x - 1, y) +
        classicNoise(x + 1, y) +
        classicNoise(x, y - 1) +
        classicNoise(x, y + 1);
    const int center = classicNoise(x, y);

    return corners / 16 + sides / 8 + center / 4;
}

int classicInterpolate(
    int a,
    int b,
    int fraction,
    int scale
) {
    int cosine =
        65536 -
        static_cast<int>(
            std::cos(
                static_cast<double>(fraction) *
                3.14159265358979323846 /
                static_cast<double>(scale)
            ) *
            65536.0
        );
    cosine >>= 1;

    return
        ((a * (65536 - cosine)) >> 16) +
        ((b * cosine) >> 16);
}

int classicInterpolatedNoise(
    int x,
    int y,
    int scale
) {
    const int scaledX = x / scale;
    const int localX = x & (scale - 1);
    const int scaledY = y / scale;
    const int localY = y & (scale - 1);

    const int v1 = classicSmoothNoise(scaledX, scaledY);
    const int v2 = classicSmoothNoise(scaledX + 1, scaledY);
    const int v3 = classicSmoothNoise(scaledX, scaledY + 1);
    const int v4 = classicSmoothNoise(scaledX + 1, scaledY + 1);

    const int i1 = classicInterpolate(v1, v2, localX, scale);
    const int i2 = classicInterpolate(v3, v4, localX, scale);
    return classicInterpolate(i1, i2, localY, scale);
}

}

int TerrainDecoder::generatedHeight(
    int worldX,
    int worldY
) {
    // The classic client offsets world coordinates before applying its
    // three-octave procedural ground-height noise.
    const int x = worldX + 932731;
    const int y = worldY + 556238;

    int height =
        classicInterpolatedNoise(x + 45365, y + 91923, 4) -
        128;
    height +=
        (
            classicInterpolatedNoise(x + 10294, y + 37821, 2) -
            128
        ) >> 1;
    height +=
        (
            classicInterpolatedNoise(x, y, 1) -
            128
        ) >> 2;

    height =
        static_cast<int>(
            static_cast<double>(height) * 0.3
        ) +
        35;

    return std::clamp(height, 10, 60);
}

MapTileArray TerrainDecoder::decode(
    const std::vector<std::uint8_t>& bytes,
    std::uint16_t regionId
) const {
    eld::binary::ByteReader reader(bytes);
    MapTileArray tiles{};

    const int baseX =
        static_cast<int>(regionId >> 8) *
        static_cast<int>(RegionSize);
    const int baseY =
        static_cast<int>(regionId & 0xFFu) *
        static_cast<int>(RegionSize);

    for (std::size_t plane = 0; plane < PlaneCount; ++plane) {
        for (std::size_t x = 0; x < RegionSize; ++x) {
            for (std::size_t y = 0; y < RegionSize; ++y) {
                MapTile& tile = tiles[tileIndex(plane, x, y)];

                bool ended = false;
                while (!ended) {
                    if (reader.atEnd()) {
                        throw std::runtime_error(
                            "Terrain data ended in the middle of a tile"
                        );
                    }

                    const std::uint8_t opcode = reader.readU8();

                    if (opcode == 0) {
                        if (plane == 0) {
                            tile.height =
                                -generatedHeight(
                                    baseX + static_cast<int>(x),
                                    baseY + static_cast<int>(y)
                                ) *
                                8;
                        }
                        else {
                            tile.height =
                                tiles[tileIndex(plane - 1, x, y)].height -
                                240;
                        }
                        ended = true;
                    }
                    else if (opcode == 1) {
                        if (reader.atEnd()) {
                            throw std::runtime_error(
                                "Explicit terrain height is missing its value"
                            );
                        }

                        std::uint8_t rawHeight = reader.readU8();
                        if (rawHeight == 1) {
                            rawHeight = 0;
                        }

                        if (plane == 0) {
                            tile.height =
                                -static_cast<int>(rawHeight) * 8;
                        }
                        else {
                            tile.height =
                                tiles[tileIndex(plane - 1, x, y)].height -
                                static_cast<int>(rawHeight) * 8;
                        }
                        ended = true;
                    }
                    else if (opcode <= 49) {
                        if (reader.atEnd()) {
                            throw std::runtime_error(
                                "Terrain overlay opcode is missing its floor id"
                            );
                        }

                        tile.overlayId = reader.readU8();
                        tile.overlayShape =
                            static_cast<std::uint8_t>((opcode - 2u) / 4u);
                        tile.overlayRotation =
                            static_cast<std::uint8_t>((opcode - 2u) & 3u);
                    }
                    else if (opcode <= 81) {
                        tile.settings =
                            static_cast<std::uint8_t>(opcode - 49u);
                    }
                    else {
                        tile.underlayId =
                            static_cast<std::uint8_t>(opcode - 81u);
                    }
                }
            }
        }
    }

    if (!reader.atEnd()) {
        throw std::runtime_error(
            "Terrain decoder left trailing bytes"
        );
    }

    return tiles;
}

}
