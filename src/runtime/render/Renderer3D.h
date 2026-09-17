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

    const opengl::SimpleRenderer::FrameStats&
    stats() const {
        return renderer_.stats();
    }

    std::uint64_t terrainDrawCalls() const {
        return terrainDrawCalls_;
    }

    std::uint64_t locationDrawCalls() const {
        return locationDrawCalls_;
    }

private:
    const ModelManager& models_;

    opengl::SimpleRenderer
        renderer_;

    std::uint64_t terrainDrawCalls_ = 0;
    std::uint64_t locationDrawCalls_ = 0;
};

}
