#pragma once

#include "backend/IRenderBackend.h"
#include "geometry/MeshProjector.h"
#include "pipeline/FaceAssembler.h"
#include "pipeline/VisibilityStage.h"
#include "scene/RenderScene.h"

namespace rf::render_next {

class RenderPipeline {
public:
    void render(
        const RenderScene& scene,
        IRenderBackend& backend
    );

private:
    MeshProjector projector_;
    FaceAssembler faceAssembler_;
    VisibilityStage visibilityStage_;
};

}
