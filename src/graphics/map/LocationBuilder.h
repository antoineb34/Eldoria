#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "location/LocationLoader.h"
#include "model/ModelLoader.h"
#include "model/ModelSystem.h"
#include "render/scene/RenderScene.h"
#include "world/Region.h"

namespace eld::graphics {

struct CameraDependentLocation {
    std::uint8_t scenePlane = 0;

    eld::render::RenderObject inset;
    eld::render::RenderObject outset;
};


struct LocationBuildResult {
    std::vector<eld::render::RenderObject> objects;

    // Classic shape-8 wall decorations select one of two
    // placements depending on camera orientation.
    std::vector<CameraDependentLocation>
        cameraDependent;

    std::size_t locations = 0;
    std::size_t modelVariants = 0;
    std::size_t missingDefinitions = 0;
    std::size_t missingModels = 0;
};


class LocationBuilder {
public:
    LocationBuildResult build(
        const eld::world::Region& region,
        std::size_t scenePlane,

        const eld::location::LocationLoader& definitions,
        eld::model::ModelLoader& models,
        eld::graphics::ModelSystem& modelSystem
    ) const;
};

}
