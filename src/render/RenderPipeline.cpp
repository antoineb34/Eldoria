#include "RenderPipeline.h"

namespace eld::render {

void RenderPipeline::render(
    const RenderScene& scene,
    const eld::graphics::GraphicsResources& resources,
    RenderBackend& backend
) const {
    backend.beginFrame(
        scene.camera
    );

    for (const RenderObject& object : scene.objects) {
        if (!object.visible) {
            continue;
        }

        backend.draw(
            object.model,
            object.transform,
            resources
        );
    }

    backend.endFrame();
}

}
