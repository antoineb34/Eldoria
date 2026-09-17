#pragma once

#include "backend/opengl/SimpleRenderer.h"
#include "model/ModelManager.h"
#include "scene/RenderScene.h"
#include "texture/TextureManager.h"

namespace eld::render {

class Renderer3D {
public:
    Renderer3D(
        const ModelManager& models,
        const TextureManager& textures
    );

    void render(
        const RenderScene& scene
    );

    void toggleWireframe();

private:
    const ModelManager& models_;

    opengl::SimpleRenderer
        renderer_;
};

}
