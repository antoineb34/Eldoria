#include "RenderPipeline.h"

namespace eld::render {

void RenderPipeline::render(
    const RenderScene& scene,
    const eld::graphics::GraphicsResources& resources,
    IRenderBackend& backend
) const {
    backend.beginFrame(
        scene.camera
    );

    for (const RenderObject& object : scene.objects) {
        if (!object.visible) {
            continue;
        }

        const eld::graphics::RenderModel& model =
            resources.getModel(
                object.model
            );

        backend.draw(
            model,
            object.transform,
            resources
        );
    }

    backend.endFrame();
}

}
