#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "render/map/ClassicTerrainAppearance.h"
#include "model/ModelManager.h"
#include "render/map/SceneLocationModelBuilder.h"
#include "floor/FloorLoader.h"
#include "model/ModelResource.h"

namespace eld::graphics {
class TextureSystem;
}

namespace eld::render::map {

struct SceneTerrainRenderStats {
  std::size_t tiles = 0;
  std::size_t triangles = 0;
  std::size_t texturedTriangles = 0;
  std::size_t drawBuckets = 0;
};

struct SceneTerrainRenderBuildResult {
  eld::render::ModelResource model;
  SceneTerrainRenderStats stats;
};

struct SceneLocationCameraRenderVariant {
  std::uint8_t scenePlane = 0;
  std::uint8_t rotation = 0;
  int sceneX = 0;
  int sceneZ = 0;

  eld::render::ModelResource insetModel;
  eld::render::ModelResource outsetModel;
};

struct SceneLocationRenderStats {
  std::size_t instances = 0;
  std::size_t parts = 0;
  std::size_t triangles = 0;
  std::size_t staticDrawBuckets = 0;
  std::size_t cameraDependentParts = 0;
};

struct SceneLocationRenderBuildResult {
  std::array<eld::render::ModelResource, eld::map::PlaneCount> staticPlaneModels;
  std::vector<SceneLocationCameraRenderVariant> cameraVariants;
  SceneLocationRenderStats stats;
};

// Converts already-decoded/classified map graphics into generic ModelResource
// resources. The output stays in classic scene units (128 units per tile), so
// callers can place/chunk it with an ordinary render::Transform.
class SceneMapRenderModelBuilder {
public:
  SceneTerrainRenderBuildResult
  buildTerrainPlane(
      std::size_t scenePlane,
      const TerrainTileSampler &sample,
      const eld::floor::FloorLoader &floors,
      eld::graphics::TextureSystem &textures) const;

  SceneLocationRenderBuildResult
  buildLocs(const SceneLocationModelBuildResult &locModels,
            const std::vector<eld::render::ModelHandle> &variantHandles,
            const eld::render::ModelManager &models) const;
};

} // namespace eld::render::map
