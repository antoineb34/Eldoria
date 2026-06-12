#pragma once

#include <SDL3/SDL.h>

#include "../IRenderBackend.h"
#include "Framebuffer.h"
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

    // Access mutable framebuffer for UI drawing
    Framebuffer& framebuffer() {
        return framebuffer_;
    }

    // Simple 2D drawing utilities for UI
    void drawRect(int x, int y, int width, int height, ColorPixel color);
    void drawRectOutline(int x, int y, int width, int height, ColorPixel color, int thickness = 1);

    void beginFrame(
        const RenderCamera& camera
    ) override;

    void drawObject(
        const RenderObject& object,
        const ProjectedMesh& mesh,
        const RenderQueue& queue
    ) override;

    void endFrame() override;

    void setHighlightTexturedFaces(
        bool enabled
    ) {
        highlightTexturedFaces_ = enabled;
    }

private:
    void destroyTexture();
    void ensureTexture();

private:
    bool highlightTexturedFaces_ = false;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;

    int viewportX_ = 0;
    int viewportY_ = 0;
    int width_ = 0;
    int height_ = 0;

    Framebuffer framebuffer_;
    TriangleRasterizer rasterizer_;
};

} // namespace eld::render
