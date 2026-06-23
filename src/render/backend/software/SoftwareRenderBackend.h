#pragma once

#include <unordered_map>

#include <SDL3/SDL.h>

#include "../IRenderBackend.h"

#include "Framebuffer.h"
#include "SoftwareMeshProjector.h"
#include "TriangleRasterizer.h"

namespace eld::render {

class SoftwareRenderBackend : public IRenderBackend {
public:
    explicit SoftwareRenderBackend(
        SDL_Renderer* renderer
    );

    ~SoftwareRenderBackend() override;

    const Framebuffer& framebuffer() const {
        return framebuffer_;
    }

    void beginFrame(
        const RenderCamera& camera
    ) override;

    void draw(
        const RenderItem& item
    ) override;

    void endFrame() override;

    void setHighlightTexturedFaces(
        bool enabled
    ) {
        highlightTexturedFaces_ =
            enabled;
    }

private:
    const SoftwareProjectedMesh& project(
        const RenderObject& object
    );

    void destroyTexture();
    void ensureTexture();

    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;

    int viewportX_ = 0;
    int viewportY_ = 0;
    int width_ = 0;
    int height_ = 0;

    bool highlightTexturedFaces_ = false;

    RenderCamera camera_;

    Framebuffer framebuffer_;
    SoftwareMeshProjector projector_;
    TriangleRasterizer rasterizer_;

    std::unordered_map<
        const RenderObject*,
        SoftwareProjectedMesh
    > projectedMeshes_;
};

}
