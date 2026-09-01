#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "SceneLocBuilder.h"
#include "definition/location/LocationRepository.h"
#include "model/ModelMesh.h"
#include "model/ModelRepository.h"

namespace eld::graphics::map {

enum class SceneLocDrawMode : std::uint8_t {
    Standard,
    WallDecorationInset,
    WallDecorationOutset,
    WallDecorationDiagonalBoth
};

struct SceneLocModelVariant {
    std::uint16_t locationId = 0;
    std::uint8_t modelType = 0;
    std::uint8_t modelRotation = 0;
    eld::model::ModelMesh mesh;
};

struct SceneLocModelPart {
    std::size_t variantIndex = 0;

    // Extra World3D draw yaw. The loc definition's model rotation has already
    // been baked into the variant mesh. 2048 units make one full turn.
    int sceneYaw = 0;

    SceneLocDrawMode drawMode =
        SceneLocDrawMode::Standard;
};

struct SceneLocModelInstance {
    std::uint16_t id = 0;
    std::uint8_t shape = 0;
    std::uint8_t rotation = 0;
    std::uint8_t scenePlane = 0;

    int sceneX = 0;
    int sceneY = 0;
    int sceneZ = 0;

    std::array<int, 4> cornerHeights{};
    bool contouredGround = false;
    bool animated = false;

    std::vector<SceneLocModelPart> parts;
};

struct SceneLocModelBuildStats {
    std::size_t placements = 0;
    std::size_t instances = 0;
    std::size_t parts = 0;
    std::size_t variants = 0;

    std::size_t missingDefinitions = 0;
    std::size_t missingShapeModels = 0;
    std::size_t missingModelFiles = 0;
    std::size_t contouredGround = 0;
    std::size_t animated = 0;
};

struct SceneLocModelBuildResult {
    std::vector<SceneLocModelVariant> variants;
    std::vector<SceneLocModelInstance> instances;
    SceneLocModelBuildStats stats;
};

class SceneLocModelBuilder {
public:
    static constexpr int FullTurn = 2048;
    static constexpr int QuarterTurn = 512;
    static constexpr int EighthTurn = 256;

    SceneLocModelBuildResult build(
        const std::vector<SceneLocPlacement>& placements,
        const eld::definition::LocationRepository& locations,
        const eld::model::ModelRepository& models
    ) const;

    // Exposed as a small deterministic conversion primitive so the cache
    // reader stays in data/model while loc-specific source transforms stay in
    // graphics/map and can be tested independently.
    static eld::model::ModelMesh transformModel(
        eld::model::ModelMesh mesh,
        const eld::definition::LocationDefinition& definition,
        int modelRotation
    );
};

}
