#pragma once

#include "../geometry/ProjectedMesh.h"
#include "RenderQueue.h"

namespace rf::render_next {

class VisibilityStage {
public:
    void apply(
        RenderQueue& queue,
        const ProjectedMesh& mesh
    ) const;
};

}
