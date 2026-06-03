#pragma once

#include "../geometry/ProjectedMesh.h"
#include "../pipeline/RenderQueue.h"
#include "../scene/RenderCamera.h"
#include "../scene/RenderObject.h"

namespace rf::render_next {

class IRenderBackend {
public:
    virtual ~IRenderBackend() = default;

    virtual void beginFrame(
        const RenderCamera& camera
    ) = 0;

    virtual void drawObject(
        const RenderObject& object,
        const ProjectedMesh& mesh,
        const RenderQueue& queue
    ) = 0;

    virtual void endFrame() = 0;
};

}
