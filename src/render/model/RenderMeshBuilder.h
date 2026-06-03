#pragma once

#include "RenderMesh.h"
#include "ModelTransform.h"

#include "../software/camera/Camera.h"

namespace rf::render {

class RenderMeshBuilder {
public:
    RenderMesh build(
        const rf::model::ModelAsset& model,
        const Camera& camera,
        const ModelTransform& transform
    );
};

}
