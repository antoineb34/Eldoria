#pragma once

#include <cstdint>

#include "views/map/MapViewState.h"
#include "floor/FloorRepository.h"
#include "location/LocationRepository.h"
#include "render/GraphicsResources.h"
#include "map/MapLoader.h"
#include "model/ModelRepository.h"

namespace eld::elforge {

class MapView {
public:
    MapView(
        const eld::map::MapLoader& loader,
        const eld::definition::FloorRepository& floors,
        const eld::definition::LocationRepository& locations,
        eld::model::ModelRepository& models,
        eld::render::GraphicsResources& graphics
    );

    MapViewState build(
        std::uint16_t regionId
    ) const;

private:
    const eld::map::MapLoader& loader_;
    const eld::definition::FloorRepository& floors_;
    const eld::definition::LocationRepository& locations_;
    eld::model::ModelRepository& models_;
    eld::render::GraphicsResources& graphics_;
};

}
