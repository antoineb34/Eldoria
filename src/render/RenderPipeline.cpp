#include "RenderPipeline.h"

namespace eld::render {

void RenderPipeline::render(
    const RenderScene& scene,
    IRenderBackend& backend
) {
    backend.beginFrame(scene.camera);

    for (int objectIndex = 0;
         objectIndex < static_cast<int>(scene.objects.size());
         objectIndex++) {
        const RenderObject& object =
            scene.objects[objectIndex];

        ProjectedMesh mesh =
            projector_.project(
                object,
                scene.camera
            );

        RenderQueue queue =
            faceAssembler_.assemble(
                objectIndex,
                object,
                mesh
            );

        visibilityStage_.apply(
            queue,
            mesh
        );

        depthSorter_.sort(queue);

        backend.drawObject(
            object,
            mesh,
            queue
        );
    }

    backend.endFrame();
}

}
