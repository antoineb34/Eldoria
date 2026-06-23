#include "RenderQueueBuilder.h"

namespace eld::render {

RenderQueue RenderQueueBuilder::build(
    const RenderScene& scene
) const {
    RenderQueue queue;

    for (
        const RenderObject& object :
        scene.objects
    ) {
        if (object.model == nullptr) {
            continue;
        }

        const RenderModel& model =
            *object.model;

        const RenderMesh& mesh =
            model.mesh;

        for (
            const RenderSubmesh& submesh :
            mesh.submeshes
        ) {
            if (
                submesh.materialIndex >=
                model.materials.size()
            ) {
                continue;
            }

            if (
                submesh.firstIndex >
                mesh.indices.size()
            ) {
                continue;
            }

            if (
                submesh.indexCount >
                mesh.indices.size() -
                    submesh.firstIndex
            ) {
                continue;
            }

            if (submesh.indexCount == 0) {
                continue;
            }

            queue.items.push_back(
                RenderItem{
                    .object = &object,
                    .submesh = &submesh,
                    .material =
                        &model.materials[
                            submesh.materialIndex
                        ],
                    .depth = 0.0f
                }
            );
        }
    }

    return queue;
}

}
