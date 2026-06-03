#include "SoftwareRenderBackend.h"

#include <vector>

#include "../../../render/software/color/Color.h"

namespace rf::render_next {

namespace {

    ColorPixel colorFromPacket(
        const RenderPacket& packet
    ) {
        rf::render::RgbColor rgb =
            rf::render::rsColorToRgb(packet.color);

        return {
            rgb.r,
            rgb.g,
            rgb.b,
            255
        };
    }

}

SoftwareRenderBackend::SoftwareRenderBackend(
    SDL_Renderer* renderer
)
    : renderer_(renderer) {
}

SoftwareRenderBackend::~SoftwareRenderBackend() {
    destroyTexture();
}

void SoftwareRenderBackend::destroyTexture() {
    if (texture_ != nullptr) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
}

void SoftwareRenderBackend::ensureTexture() {
    if (texture_ != nullptr) {
        return;
    }

    texture_ =
        SDL_CreateTexture(
            renderer_,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STREAMING,
            width_,
            height_
        );
}

void SoftwareRenderBackend::beginFrame(
    const RenderCamera& camera
) {
    viewportX_ = camera.viewportX;
    viewportY_ = camera.viewportY;
    width_ = camera.viewportWidth;
    height_ = camera.viewportHeight;

    framebuffer_.resize(
        width_,
        height_
    );

    framebuffer_.clear();

    destroyTexture();
    ensureTexture();
}

void SoftwareRenderBackend::drawObject(
    const RenderObject& object,
    const ProjectedMesh& mesh,
    const RenderQueue& queue
) {
    (void) object;

    for (const RenderPacket& packet : queue.packets) {
        rf::render::ScreenPoint a =
            mesh.vertices[packet.a].screen;

        rf::render::ScreenPoint b =
            mesh.vertices[packet.b].screen;

        rf::render::ScreenPoint c =
            mesh.vertices[packet.c].screen;

        rasterizer_.drawSolidTriangle(
            framebuffer_,
            a,
            b,
            c,
            colorFromPacket(packet)
        );
    }
}

void SoftwareRenderBackend::endFrame() {
    if (
        renderer_ == nullptr ||
        texture_ == nullptr ||
        width_ <= 0 ||
        height_ <= 0
    ) {
        return;
    }

    std::vector<ColorPixel> pixels;
    pixels.reserve(width_ * height_);

    for (int y = 0; y < height_; y++) {
        for (int x = 0; x < width_; x++) {
            pixels.push_back(
                framebuffer_.color().at(x, y)
            );
        }
    }

    SDL_UpdateTexture(
        texture_,
        nullptr,
        pixels.data(),
        width_ * static_cast<int>(sizeof(ColorPixel))
    );

    SDL_FRect destination {
        static_cast<float>(viewportX_),
        static_cast<float>(viewportY_),
        static_cast<float>(width_),
        static_cast<float>(height_)
    };

    SDL_RenderTexture(
        renderer_,
        texture_,
        nullptr,
        &destination
    );
}

}
