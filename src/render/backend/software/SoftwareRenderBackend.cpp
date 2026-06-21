#include "SoftwareRenderBackend.h"

#include <vector>

#include "../../../render/color/Color.h"

namespace eld::render {

namespace {

    ColorPixel colorFromPacket(
        const RenderPacket& packet
    ) {
        eld::render::RgbColor rgb =
            eld::render::rsColorToRgb(packet.color);

        uint8_t alpha = 255;

        if (packet.alpha > 0) {
            alpha = static_cast<uint8_t>(255 - packet.alpha);
        }

        return {
            rgb.r,
            rgb.g,
            rgb.b,
            alpha
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
    for (const RenderPacket& packet : queue.packets) {
        eld::render::ScreenPoint a =
            mesh.vertices[packet.a].screen;

        eld::render::ScreenPoint b =
            mesh.vertices[packet.b].screen;

        eld::render::ScreenPoint c =
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
