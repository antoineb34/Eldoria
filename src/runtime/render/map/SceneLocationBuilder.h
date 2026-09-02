#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include "location/LocationRepository.h"
#include "map/MapRegion.h"
#include "map/MapTile.h"

namespace eld::render::map {

enum class SceneLocationKind : std::uint8_t {
    Wall,
    WallDecoration,
    GroundDecoration,
    Location,
    Roof
};

struct SceneLocationPlacement {
    std::uint16_t id = 0;
    std::uint8_t shape = 0;
    std::uint8_t rotation = 0;
    std::uint8_t modelType = 0;

    std::uint8_t sourcePlane = 0;
    std::uint8_t scenePlane = 0;
    bool bridgeAttachment = false;

    int tileX = 0;
    int tileZ = 0;
    int footprintWidth = 1;
    int footprintLength = 1;

    // Classic World3D scene coordinates: 128 units per map tile.
    int sceneX = 0;
    int sceneZ = 0;
    int sceneY = 0;

    // Rotation passed into LocType::getModel is a quarter-turn count, not a
    // World3D yaw. L walls request two independently rotated model instances.
    std::uint8_t primaryModelRotation = 0;
    std::uint8_t secondaryModelRotation = 0;
    bool hasSecondaryModel = false;

    // Extra World3D yaw applied after the model is built. This is normally 0;
    // centrepiece-diagonal (shape 11) uses the classic extra 256 (45 degrees).
    int sceneYaw = 0;

    // Exact scene metadata used by wall/decor placement. These are useful to
    // verify placement now and to drive real model placement in the next pass.
    int wallTypeA = 0;
    int wallTypeB = 0;
    int decorationType = 0;
    int decorationAngle = 0;
    int decorationOffsetX = 0;
    int decorationOffsetZ = 0;

    std::array<int, 4> cornerHeights{};
    SceneLocationKind kind = SceneLocationKind::Location;
};

using SceneLocationTileSampler =
    std::function<const eld::map::MapTile*(std::size_t, int, int)>;

class SceneLocationBuilder {
public:
    static constexpr int TileSize = 128;
    static constexpr int FullTurn = 2048;
    static constexpr int QuarterTurn = 512;
    static constexpr int EighthTurn = 256;

    static constexpr std::uint8_t normalizedModelType(
        std::uint8_t shape
    ) {
        if (shape == 11) {
            return 10;
        }

        if (shape >= 5 && shape <= 8) {
            return 4;
        }

        return shape;
    }

    std::vector<SceneLocationPlacement> build(
        const std::vector<eld::map::MapLocationSpawn>& spawns,
        const eld::definition::LocationRepository& locations,
        const SceneLocationTileSampler& sampleTile
    ) const;
};

}
