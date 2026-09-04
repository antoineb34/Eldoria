#pragma once

#include <cstdint>

#include "views/map/MapViewState.h"
#include "repositories/FloorRepository.h"
#include "repositories/LocationRepository.h"
#include "render/GraphicsResources.h"
#include "map/MapRepository.h"
#include "repositories/ModelRepository.h"

namespace eld::elforge {

class MapView {
public:
    MapView(
        const eld::map::MapRepository& loader,
        const eld::floor::FloorRepository& floors,
        const eld::location::LocationRepository& locations,
        eld::model::ModelRepository& models,
        eld::render::GraphicsResources& graphics
    );

    MapViewState build(
        std::uint16_t regionId
    ) const;

private:
    const eld::map::MapRepository& loader_;
    const eld::floor::FloorRepository& floors_;
    const eld::location::LocationRepository& locations_;
    eld::model::ModelRepository& models_;
    eld::render::GraphicsResources& graphics_;
};

}
