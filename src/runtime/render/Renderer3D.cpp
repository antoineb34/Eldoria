#include "Renderer3D.h"

namespace eld::render {

Renderer3D::Renderer3D(
    const ModelManager& models,
    const TextureManager& textures
)
    : models_(models),
      renderer_(textures)
{
}


void Renderer3D::render(
    const RenderScene& scene
)
{
    renderer_.beginFrame(
        scene.camera
    );

    for (
        const RenderObject& object :
        scene.objects
    ) {
        if (!object.visible) {
            continue;
        }

        if (
            !models_.isValid(
                object.model
            )
        ) {
            continue;
        }

        const ModelResource& model =
            models_.get(
                object.model
            );

        renderer_.draw(
            object.model,
            model,
            object.transform
        );
    }

    renderer_.endFrame();
}


void Renderer3D::toggleWireframe()
{
    renderer_.toggleWireframe();
}

}
