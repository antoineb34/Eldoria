#include "RenderPipeline.h"

namespace eld::render {

void RenderPipeline::render(
    const RenderScene& scene,
    IRenderBackend& backend
) {
    backend.beginFrame(
        scene.camera
    );

    const RenderQueue queue =
        queueBuilder_.build(
            scene
        );

    for (
        const RenderItem& item :
        queue.items
    ) {
        backend.draw(
            item
        );
    }

    backend.endFrame();
}

}
