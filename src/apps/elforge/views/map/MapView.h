#pragma once

#include <cstdint>

#include "floor/FloorLoader.h"
#include "location/LocationLoader.h"
#include "map/MapLoader.h"
#include "model/ModelLoader.h"
#include "model/ModelSystem.h"
#include "texture/TextureSystem.h"
#include "render/model/ModelManager.h"
#include "views/map/MapViewState.h"

namespace eld::elforge {

class MapView {
public:
  MapView(const eld::map::MapLoader &loader,
          const eld::floor::FloorLoader &floors,
          const eld::location::LocationLoader &locations,
          eld::model::ModelLoader &models,
          eld::graphics::ModelSystem &modelSystem,
          eld::graphics::TextureSystem &textureSystem,
          eld::render::ModelManager &modelManager);

  MapViewState build(std::uint16_t regionId) const;

private:
  const eld::map::MapLoader &loader_;
  const eld::floor::FloorLoader &floors_;
  const eld::location::LocationLoader &locations_;
  eld::model::ModelLoader &models_;
  eld::graphics::ModelSystem &modelSystem_;
  eld::graphics::TextureSystem &textureSystem_;
  eld::render::ModelManager &modelManager_;
};

} // namespace eld::elforge
