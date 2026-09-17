#pragma once

#include <cstddef>
#include <vector>

#include "render/model/ModelManager.h"
#include "render/model/ModelResource.h"
#include "render/scene/RenderObject.h"

namespace eld::graphics {

struct LocationBatchBuildResult {
    std::vector<eld::render::ModelResource>
        batches;

    std::vector<eld::render::RenderObject>
        passthroughObjects;

    std::size_t sourceObjects = 0;
    std::size_t batchedObjects = 0;
    std::size_t batchSections = 0;
};

class LocationBatchBuilder {
public:
    LocationBatchBuildResult build(
        const std::vector<
            eld::render::RenderObject
        >& objects,

        const eld::render::ModelManager&
            models
    ) const;
};

}
