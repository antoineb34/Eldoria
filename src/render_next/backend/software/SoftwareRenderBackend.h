#pragma once

#include <SDL3/SDL.h>

#include "../IRenderBackend.h"
#include "Framebuffer.h"
#include "TriangleRasterizer.h"

namespace rf::render_next {

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

    void drawObject(
        const RenderObject& object,
        const ProjectedMesh& mesh,
        const RenderQueue& queue
    ) override;

    void endFrame() override;

private:
    void destroyTexture();
    void ensureTexture();

private:
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;

    int viewportX_ = 0;
    int viewportY_ = 0;
    int width_ = 0;
    int height_ = 0;

    Framebuffer framebuffer_;
    TriangleRasterizer rasterizer_;
};

}
