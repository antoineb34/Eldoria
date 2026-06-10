#pragma once

#include "../geometry/ProjectedMesh.h"
#include "../scene/RenderObject.h"
#include "RenderQueue.h"

namespace eld::render {

class FaceAssembler {
public:
    RenderQueue assemble(
        int objectIndex,
        const RenderObject& object,
        const ProjectedMesh& mesh
    ) const;
};

}
