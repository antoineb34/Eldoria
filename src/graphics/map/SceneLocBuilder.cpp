#include "SceneLocBuilder.h"

#include <array>
#include <cstddef>
#include <optional>

#include "ClassicTileRules.h"

namespace eld::graphics::map {
namespace {

constexpr std::array<int, 4> RotationWallType{
    1, 2, 4, 8
};

constexpr std::array<int, 4> RotationWallCornerType{
    16, 32, 64, 128
};

constexpr std::array<int, 4> WallDecorationForwardX{
    1, 0, -1, 0
};

constexpr std::array<int, 4> WallDecorationForwardZ{
    0, -1, 0, 1
};

constexpr std::size_t WallOffsetPlaneStride =
    eld::map::RegionSize * eld::map::RegionSize;
constexpr std::size_t WallOffsetCount =
    eld::map::PlaneCount * WallOffsetPlaneStride;

std::size_t wallOffsetIndex(
    std::size_t plane,
    std::size_t x,
    std::size_t z
) {
    return
        plane * WallOffsetPlaneStride +
        x * eld::map::RegionSize +
        z;
}

SceneLocKind kindForShape(std::uint8_t shape) {
    if (shape <= 3) {
        return SceneLocKind::Wall;
    }

    if (shape >= 4 && shape <= 8) {
        return SceneLocKind::WallDecoration;
    }

    if (shape >= 12 && shape <= 21) {
        return SceneLocKind::Roof;
    }

    if (shape == 22) {
        return SceneLocKind::GroundDecoration;
    }

    return SceneLocKind::Location;
}


}

std::vector<SceneLocPlacement> SceneLocBuilder::build(
    const std::vector<eld::map::MapObjectSpawn>& spawns,
    const eld::definition::LocationRepository& locations,
    const SceneLocTileSampler& sampleTile
) const {
    std::vector<SceneLocPlacement> result;
    result.reserve(spawns.size());

    // World3D starts wall-decoration displacement at 16. Straight and L walls
    // can replace it for later type-5 wall decorations on the same tile.
    std::array<std::uint8_t, WallOffsetCount> wallOffsets{};
    wallOffsets.fill(16);

    for (const eld::map::MapObjectSpawn& spawn : spawns) {
        if (
            spawn.plane >= eld::map::PlaneCount ||
            spawn.x >= eld::map::RegionSize ||
            spawn.y >= eld::map::RegionSize ||
            spawn.type > 22 ||
            spawn.rotation > 3
        ) {
            continue;
        }

        const eld::definition::LocationDefinition* definition =
            locations.find(spawn.id);
        if (definition == nullptr) {
            continue;
        }

        const int x = static_cast<int>(spawn.x);
        const int z = static_cast<int>(spawn.y);
        const std::size_t plane = spawn.plane;

        const eld::map::MapTile* southwest =
            sampleTile(plane, x, z);
        const eld::map::MapTile* southeast =
            sampleTile(plane, x + 1, z);
        const eld::map::MapTile* northeast =
            sampleTile(plane, x + 1, z + 1);
        const eld::map::MapTile* northwest =
            sampleTile(plane, x, z + 1);

        if (
            southwest == nullptr || southeast == nullptr ||
            northeast == nullptr || northwest == nullptr
        ) {
            continue;
        }

        const eld::map::MapTile* levelOne =
            sampleTile(1, x, z);
        const std::uint8_t levelOneSettings =
            levelOne == nullptr ? 0 : levelOne->settings;

        const std::optional<std::size_t> scenePlane =
            ClassicTileRules::scenePlaneForSourcePlane(
                spawn.plane,
                levelOneSettings
            );
        if (!scenePlane.has_value()) {
            continue;
        }

        SceneLocPlacement placement;
        placement.id = spawn.id;
        placement.shape = spawn.type;
        placement.rotation = spawn.rotation;
        placement.modelType = normalizedModelType(spawn.type);
        placement.sourcePlane = spawn.plane;
        placement.scenePlane = static_cast<std::uint8_t>(*scenePlane);
        placement.bridgeAttachment =
            spawn.plane == 0 &&
            ClassicTileRules::isBridge(levelOneSettings);
        placement.tileX = x;
        placement.tileZ = z;
        placement.kind = kindForShape(spawn.type);
        placement.cornerHeights = {
            southwest->height,
            southeast->height,
            northeast->height,
            northwest->height
        };
        placement.sceneY =
            (
                southwest->height + southeast->height +
                northeast->height + northwest->height
            ) >> 2;

        const int rotation = static_cast<int>(spawn.rotation);
        placement.primaryModelRotation = spawn.rotation;
        placement.sceneYaw = 0;

        if (spawn.type == 10 || spawn.type == 11) {
            int width = definition->width;
            int length = definition->length;

            if (spawn.rotation == 1 || spawn.rotation == 3) {
                const int oldWidth = width;
                width = length;
                length = oldWidth;
            }

            placement.footprintWidth = width;
            placement.footprintLength = length;
            placement.sceneX = x * TileSize + width * (TileSize / 2);
            placement.sceneZ = z * TileSize + length * (TileSize / 2);

            if (spawn.type == 11) {
                placement.sceneYaw = EighthTurn;
            }
        }
        else {
            // Walls, wall decorations, type 9, roofs 12..21 and ground decor
            // are all inserted into World3D on one scene tile.
            placement.footprintWidth = 1;
            placement.footprintLength = 1;
            placement.sceneX = x * TileSize + TileSize / 2;
            placement.sceneZ = z * TileSize + TileSize / 2;
        }

        if (spawn.type <= 3) {
            if (spawn.type == 0) {
                placement.wallTypeA = RotationWallType[spawn.rotation];
            }
            else if (spawn.type == 1 || spawn.type == 3) {
                placement.wallTypeA =
                    RotationWallCornerType[spawn.rotation];
            }
            else {
                const std::uint8_t nextRotation =
                    static_cast<std::uint8_t>((spawn.rotation + 1u) & 3u);
                placement.wallTypeA = RotationWallType[spawn.rotation];
                placement.wallTypeB = RotationWallType[nextRotation];
                placement.primaryModelRotation =
                    static_cast<std::uint8_t>(spawn.rotation + 4u);
                placement.secondaryModelRotation = nextRotation;
                placement.hasSecondaryModel = true;
            }
        }
        else if (spawn.type >= 4 && spawn.type <= 8) {
            placement.primaryModelRotation = 0;

            if (spawn.type == 4 || spawn.type == 5) {
                placement.decorationType = RotationWallType[spawn.rotation];
                placement.decorationAngle = rotation * QuarterTurn;
            }
            else if (spawn.type == 6) {
                placement.decorationType = 256;
                placement.decorationAngle = rotation;
            }
            else if (spawn.type == 7) {
                placement.decorationType = 512;
                placement.decorationAngle = rotation;
            }
            else {
                placement.decorationType = 768;
                placement.decorationAngle = rotation;
            }

            if (spawn.type == 5) {
                const std::uint8_t offset =
                    wallOffsets[wallOffsetIndex(
                        plane,
                        spawn.x,
                        spawn.y
                    )];

                placement.decorationOffsetX =
                    WallDecorationForwardX[spawn.rotation] *
                    static_cast<int>(offset);
                placement.decorationOffsetZ =
                    WallDecorationForwardZ[spawn.rotation] *
                    static_cast<int>(offset);
                placement.sceneX += placement.decorationOffsetX;
                placement.sceneZ += placement.decorationOffsetZ;
            }
        }

        result.push_back(placement);

        // World::addLoc calls setWallDecorationOffset after straight and L
        // walls. Preserve object-stream order so a type-5 decoration sees the
        // same wall displacement the classic client would have seen.
        if (
            (spawn.type == 0 || spawn.type == 2) &&
            definition->decorDisplacement != 16
        ) {
            wallOffsets[wallOffsetIndex(
                plane,
                spawn.x,
                spawn.y
            )] = definition->decorDisplacement;
        }
    }

    return result;
}

}
