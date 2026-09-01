#pragma once

#include <cstdint>

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

    void beginFrame(
        const Camera& camera
    ) override;

    void draw(
        eld::graphics::ModelHandle model,
        const Transform& transform,
        const eld::graphics::GraphicsResources& resources
    ) override;

    void endFrame() override;

    void setOutputPosition(
        int x,
        int y
    );

    void setClearColor(
        ColorPixel color
    );

    const Framebuffer& framebuffer() const;

private:
    void destroyTexture();
    void ensureTexture();

    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;

    int outputX_ = 0;
    int outputY_ = 0;

    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;

    ColorPixel clearColor_{
        0,
        0,
        0,
        255
    };

    Camera camera_;

    Framebuffer framebuffer_;
    SoftwareMeshProjector projector_;
    TriangleRasterizer rasterizer_;
};

}
