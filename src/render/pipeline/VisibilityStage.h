#pragma once

#include "../geometry/ProjectedMesh.h"
#include "RenderQueue.h"

namespace eld::render {

class VisibilityStage {
public:
    void apply(
        RenderQueue& queue,
        const ProjectedMesh& mesh
    ) const;
};

}
