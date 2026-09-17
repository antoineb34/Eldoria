#include "RenderPipeline.h"

namespace eld::render {

void RenderPipeline::render(
    const RenderScene& scene,
    const ModelManager& models,
    const TextureManager& textures,
    RenderBackend& backend
) const {
    backend.beginFrame(scene.camera);

    for (const RenderObject& object : scene.objects) {
        if (!object.visible) {
            continue;
        }

        const ModelResource& model =
            models.get(object.model);

        backend.draw(
            object.model,
            model,
            object.transform,
            textures
        );
    }

    backend.endFrame();
}

}
