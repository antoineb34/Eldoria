#include "RegionBuilder.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

#include "map/MapTile.h"

namespace eld::runtime::map {

namespace {

constexpr float CacheHeightScale =
    -1.0f / 128.0f;

constexpr std::array<int, 4> WallDecorationForwardX{
    1, 0, -1, 0
};

constexpr std::array<int, 4> WallDecorationForwardZ{
    0, -1, 0, 1
};


eld::world::TerrainOrigin regionOrigin(
    std::uint16_t regionId
) {
    return {
        static_cast<int>(regionId >> 8) *
            static_cast<int>(eld::map::RegionSize),

        static_cast<int>(regionId & 0xFFu) *
            static_cast<int>(eld::map::RegionSize)
    };
}


std::optional<eld::world::FloorId> floorId(
    std::uint8_t rawId
) {
    if (rawId == 0) {
        return std::nullopt;
    }

    return static_cast<eld::world::FloorId>(
        rawId - 1
    );
}


eld::world::QuarterTurn rotation(
    std::uint8_t value
) {
    return static_cast<eld::world::QuarterTurn>(
        value & 3u
    );
}


std::uint8_t scenePlane(
    std::size_t sourcePlane,
    std::uint8_t levelOneSettings
) {
    const bool bridge =
        eld::map::hasTileFlag(
            levelOneSettings,
            eld::map::TileFlag::Bridge
        );

    if (bridge && sourcePlane > 0) {
        return static_cast<std::uint8_t>(
            sourcePlane - 1
        );
    }

    return static_cast<std::uint8_t>(
        sourcePlane
    );
}


std::size_t tileIndex(
    std::size_t plane,
    std::size_t x,
    std::size_t y
) {
    constexpr std::size_t Size =
        eld::map::RegionSize;

    return
        plane * Size * Size +
        x * Size +
        y;
}


std::size_t heightIndex(
    std::size_t plane,
    std::size_t x,
    std::size_t y
) {
    constexpr std::size_t Size =
        eld::map::RegionSize + 1;

    return
        plane * Size * Size +
        x * Size +
        y;
}


// Get a height sample.
//
// x/y may equal RegionSize because Terrain stores
// a 65x65 corner grid for a 64x64 tile grid.
//
// When that happens we use the neighboring region.
// If it is unavailable, clamp to our own edge.
int mapHeight(
    const eld::map::MapLoader& maps,
    const eld::map::TerrainData& center,
    std::uint16_t regionId,
    std::size_t plane,
    std::size_t x,
    std::size_t y
) {
    constexpr std::size_t Size =
        eld::map::RegionSize;

    const int baseRegionX =
        static_cast<int>(regionId >> 8);

    const int baseRegionY =
        static_cast<int>(regionId & 0xFFu);

    const int targetRegionX =
        baseRegionX +
        static_cast<int>(x / Size);

    const int targetRegionY =
        baseRegionY +
        static_cast<int>(y / Size);

    const std::size_t localX =
        x % Size;

    const std::size_t localY =
        y % Size;


    if (
        targetRegionX >= 0 &&
        targetRegionX <= 255 &&
        targetRegionY >= 0 &&
        targetRegionY <= 255
    ) {
        const auto targetRegionId =
            static_cast<std::uint16_t>(
                (targetRegionX << 8) |
                targetRegionY
            );

        if (maps.contains(targetRegionId)) {
            return maps
                .terrain(targetRegionId)
                .tile(
                    plane,
                    localX,
                    localY
                )
                .height;
        }
    }


    return center
        .tile(
            plane,
            std::min(x, Size - 1),
            std::min(y, Size - 1)
        )
        .height;
}


eld::world::Terrain buildTerrain(
    std::uint16_t regionId,
    const eld::map::MapLoader& maps
) {
    constexpr std::size_t Size =
        eld::map::RegionSize;

    constexpr std::size_t Planes =
        eld::map::PlaneCount;

    constexpr std::size_t HeightSize =
        Size + 1;


    const auto origin =
        regionOrigin(regionId);

    const auto& source =
        maps.terrain(regionId);


    // --------------------------------------------------------
    // Shared corner heights
    // --------------------------------------------------------

    std::vector<float> heights(
        Planes *
        HeightSize *
        HeightSize
    );


    for (
        std::size_t plane = 0;
        plane < Planes;
        ++plane
    ) {
        for (
            std::size_t x = 0;
            x <= Size;
            ++x
        ) {
            for (
                std::size_t y = 0;
                y <= Size;
                ++y
            ) {
                heights[
                    heightIndex(
                        plane,
                        x,
                        y
                    )
                ] =
                    static_cast<float>(
                        mapHeight(
                            maps,
                            source,
                            regionId,
                            plane,
                            x,
                            y
                        )
                    ) *
                    CacheHeightScale;
            }
        }
    }


    // --------------------------------------------------------
    // Tiles
    // --------------------------------------------------------

    std::vector<eld::world::Tile> tiles(
        Planes *
        Size *
        Size
    );


    for (
        std::size_t sourcePlane = 0;
        sourcePlane < Planes;
        ++sourcePlane
    ) {
        for (
            std::size_t x = 0;
            x < Size;
            ++x
        ) {
            for (
                std::size_t y = 0;
                y < Size;
                ++y
            ) {
                const auto& sourceTile =
                    source.tile(
                        sourcePlane,
                        x,
                        y
                    );

                const auto levelOneSettings =
                    source.tile(
                        1,
                        x,
                        y
                    ).settings;


                eld::world::Tile tile{};

                tile.surface.underlay =
                    floorId(
                        sourceTile.underlayId
                    );

                tile.surface.overlay =
                    floorId(
                        sourceTile.overlayId
                    );


                if (tile.surface.overlay) {
                    tile.surface.shape =
                        static_cast<std::uint8_t>(
                            sourceTile.overlayShape +
                            1u
                        );

                    tile.surface.rotation =
                        rotation(
                            sourceTile.overlayRotation
                        );
                }


                tile.flags.solid =
                    sourceTile.hasFlag(
                        eld::map::TileFlag::Solid
                    );

                tile.flags.bridge =
                    sourceTile.hasFlag(
                        eld::map::TileFlag::Bridge
                    );

                tile.flags.roof =
                    sourceTile.hasFlag(
                        eld::map::TileFlag::Roof
                    );


                tile.sourcePlane =
                    static_cast<std::uint8_t>(
                        sourcePlane
                    );

                tile.scenePlane =
                    scenePlane(
                        sourcePlane,
                        levelOneSettings
                    );


                tiles[
                    tileIndex(
                        sourcePlane,
                        x,
                        y
                    )
                ] =
                    tile;
            }
        }
    }


    return eld::world::Terrain(
        origin,
        Size,
        Size,
        Planes,
        std::move(heights),
        std::move(tiles)
    );
}


std::vector<eld::world::Location>
buildLocations(
    std::uint16_t regionId,
    const eld::map::MapLoader& maps,
    const eld::location::LocationLoader& definitions,
    const eld::world::Terrain& terrain
) {
    constexpr std::size_t Size =
        eld::map::RegionSize;

    constexpr std::size_t PlaneStride =
        Size * Size;


    const auto origin =
        regionOrigin(regionId);

    const auto& sourceTerrain =
        maps.terrain(regionId);

    const auto& spawns =
        maps.locations(regionId);


    std::vector<eld::world::Location> result;
    result.reserve(spawns.size());


    // Wall decoration type 5 depends on the displacement
    // of walls encountered earlier in the spawn stream.
    std::array<
        std::uint8_t,
        eld::map::PlaneCount * PlaneStride
    > wallOffsets{};

    wallOffsets.fill(16);


    const auto wallOffsetIndex = [](
        std::size_t plane,
        std::size_t x,
        std::size_t y
    ) {
        return
            plane * PlaneStride +
            x * Size +
            y;
    };


    for (const auto& spawn : spawns) {
        if (
            spawn.plane >= eld::map::PlaneCount ||
            spawn.x >= Size ||
            spawn.y >= Size ||
            spawn.rotation > 3
        ) {
            continue;
        }


        const auto definition =
            definitions.find(spawn.id);

        const auto levelOneSettings =
            sourceTerrain.tile(
                1,
                spawn.x,
                spawn.y
            ).settings;


        // ----------------------------------------------------
        // Footprint
        // ----------------------------------------------------

        int width = 1;
        int length = 1;


        if (
            definition &&
            (
                spawn.type == 10 ||
                spawn.type == 11
            )
        ) {
            width =
                static_cast<int>(
                    definition->width
                );

            length =
                static_cast<int>(
                    definition->length
                );


            if (
                spawn.rotation == 1 ||
                spawn.rotation == 3
            ) {
                std::swap(
                    width,
                    length
                );
            }
        }


        // ----------------------------------------------------
        // Terrain beneath this location
        // ----------------------------------------------------

        const auto terrainPosition =
            terrain.layerPosition(
                spawn.plane,
                spawn.x,
                spawn.y
            );

        const auto ground =
            terrain.cornerHeights(
                terrainPosition
            );


        // ----------------------------------------------------
        // World location
        // ----------------------------------------------------

        eld::world::Location location{};

        location.id =
            spawn.id;

        location.shape =
            spawn.type;

        location.rotation =
            rotation(
                spawn.rotation
            );

        location.footprintWidth =
            width;

        location.footprintLength =
            length;


        location.solid =
            definition &&
            definition->solid &&
            !definition->hollow;


        location.tile = {
            origin.x +
                static_cast<int>(spawn.x),

            origin.y +
                static_cast<int>(spawn.y),

            static_cast<int>(
                scenePlane(
                    spawn.plane,
                    levelOneSettings
                )
            )
        };


        float x =
            static_cast<float>(
                location.tile.x
            ) +
            static_cast<float>(width) *
                0.5f;

        float z =
            static_cast<float>(
                location.tile.y
            ) +
            static_cast<float>(length) *
                0.5f;


        // Type-5 wall decoration displacement is part
        // of the resolved World position.
        if (spawn.type == 5) {
            const int displacement =
                static_cast<int>(
                    wallOffsets[
                        wallOffsetIndex(
                            spawn.plane,
                            spawn.x,
                            spawn.y
                        )
                    ]
                );

            x +=
                static_cast<float>(
                    WallDecorationForwardX[
                        spawn.rotation
                    ] *
                    displacement
                ) /
                128.0f;

            z +=
                static_cast<float>(
                    WallDecorationForwardZ[
                        spawn.rotation
                    ] *
                    displacement
                ) /
                128.0f;
        }


        location.position = {
            x,
            ground.average(),
            z
        };


        location.groundHeights = {
            ground.southwest,
            ground.southeast,
            ground.northeast,
            ground.northwest
        };


        result.push_back(location);


        // Straight/L walls affect later type-5 wall
        // decorations on the same tile.
        if (
            definition &&
            (
                spawn.type == 0 ||
                spawn.type == 2
            ) &&
            definition->decorDisplacement != 16
        ) {
            wallOffsets[
                wallOffsetIndex(
                    spawn.plane,
                    spawn.x,
                    spawn.y
                )
            ] =
                definition->decorDisplacement;
        }
    }


    return result;
}

}


eld::world::Region
RegionBuilder::build(
    std::uint16_t regionId,
    const eld::map::MapLoader& maps,
    const eld::location::LocationLoader& locations
) const {
    auto terrain =
        buildTerrain(
            regionId,
            maps
        );

    auto worldLocations =
        buildLocations(
            regionId,
            maps,
            locations,
            terrain
        );

    return {
        regionId,
        std::move(terrain),
        std::move(worldLocations)
    };
}

}
