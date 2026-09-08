#pragma once

#include <cstdint>

#include "floor/FloorLoader.h"
#include "location/LocationLoader.h"
#include "model/ModelLoader.h"
#include "render/GraphicsResources.h"
#include "repositories/MapRepository.h"
#include "views/map/MapViewState.h"

namespace eld::elforge {

class MapView {
public:
  MapView(const eld::map::MapRepository &loader,
          const eld::floor::FloorLoader &floors,
          const eld::location::LocationLoader &locations,
          eld::model::ModelLoader &models,
          eld::render::GraphicsResources &graphics);

  MapViewState build(std::uint16_t regionId) const;

private:
  const eld::map::MapRepository &loader_;
  const eld::floor::FloorLoader &floors_;
  const eld::location::LocationLoader &locations_;
  eld::model::ModelLoader &models_;
  eld::render::GraphicsResources &graphics_;
};

} // namespace eld::elforge
