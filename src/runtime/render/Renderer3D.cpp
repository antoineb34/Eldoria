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

    terrainDrawCalls_ = 0;
    locationDrawCalls_ = 0;

    std::size_t objectIndex = 0;

    for (
        const RenderObject& object :
        scene.objects
    ) {
        if (!object.visible) {
            ++objectIndex;
            continue;
        }

        if (
            !models_.isValid(
                object.model
            )
        ) {
            ++objectIndex;
            continue;
        }

        const ModelResource& model =
            models_.get(
                object.model
            );

        const auto before =
            renderer_.stats().drawCalls;

        renderer_.draw(
            object.model,
            model,
            object.transform
        );

        const auto after =
            renderer_.stats().drawCalls;

        const auto objectDraws =
            after - before;

        if (objectIndex == 0) {
            terrainDrawCalls_ =
                objectDraws;
        } else {
            locationDrawCalls_ +=
                objectDraws;
        }

        ++objectIndex;
    }

    renderer_.endFrame();
}


void Renderer3D::toggleWireframe()
{
    renderer_.toggleWireframe();
}

}
