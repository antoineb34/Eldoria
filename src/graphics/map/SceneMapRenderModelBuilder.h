#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "ClassicTerrainAppearance.h"
#include "SceneLocationModelBuilder.h"
#include "GraphicsResources.h"
#include "definition/floor/FloorRepository.h"
#include "model/RenderModel.h"

namespace eld::graphics::map {

struct SceneTerrainRenderStats {
    std::size_t tiles = 0;
    std::size_t triangles = 0;
    std::size_t texturedTriangles = 0;
    std::size_t drawBuckets = 0;
};

struct SceneTerrainRenderBuildResult {
    eld::graphics::RenderModel model;
    SceneTerrainRenderStats stats;
};

struct SceneLocationCameraRenderVariant {
    std::uint8_t scenePlane = 0;
    std::uint8_t rotation = 0;
    int sceneX = 0;
    int sceneZ = 0;

    eld::graphics::RenderModel insetModel;
    eld::graphics::RenderModel outsetModel;
};

struct SceneLocationRenderStats {
    std::size_t instances = 0;
    std::size_t parts = 0;
    std::size_t triangles = 0;
    std::size_t staticDrawBuckets = 0;
    std::size_t cameraDependentParts = 0;
};

struct SceneLocationRenderBuildResult {
    std::array<eld::graphics::RenderModel, eld::map::PlaneCount>
        staticPlaneModels;
    std::vector<SceneLocationCameraRenderVariant> cameraVariants;
    SceneLocationRenderStats stats;
};

// Converts already-decoded/classified map graphics into generic RenderModel
// resources. The output stays in classic scene units (128 units per tile), so
// callers can place/chunk it with an ordinary render::Transform.
class SceneMapRenderModelBuilder {
public:
    SceneTerrainRenderBuildResult buildTerrainPlane(
        std::size_t scenePlane,
        const TerrainTileSampler& sample,
        const eld::definition::FloorRepository& floors,
        eld::graphics::GraphicsResources& resources
    ) const;

    SceneLocationRenderBuildResult buildLocs(
        const SceneLocationModelBuildResult& locModels,
        const std::vector<eld::graphics::ModelHandle>& variantHandles,
        const eld::graphics::GraphicsResources& resources
    ) const;
};

}
