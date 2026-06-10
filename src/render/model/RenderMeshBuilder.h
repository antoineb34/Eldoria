#pragma once

#include "RenderMesh.h"
#include "ModelTransform.h"

#include "../software/camera/Camera.h"

namespace eld::render {

class RenderMeshBuilder {
public:
    RenderMesh build(
        const eld::model::ModelAsset& model,
        const Camera& camera,
        const ModelTransform& transform
    );
};

}
