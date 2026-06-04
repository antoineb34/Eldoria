#pragma once

#include "../scene/RenderCamera.h"
#include "../scene/RenderObject.h"
#include "ProjectedMesh.h"

namespace rf::render_next {

class MeshProjector {
public:
    ProjectedMesh project(
        const RenderObject& object,
        const RenderCamera& camera
    ) const;
};

}
