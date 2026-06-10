#pragma once

#include "../scene/RenderCamera.h"
#include "../scene/RenderObject.h"
#include "ProjectedMesh.h"

namespace eld::render_next {

class MeshProjector {
public:
    ProjectedMesh project(
        const RenderObject& object,
        const RenderCamera& camera
    ) const;
};

}
