#include "SoftwareRenderBackend.h"

#include <vector>

#include "../../../render/color/Color.h"

namespace eld::render {

namespace {

    bool isTexturedPacket(
        const RenderPacket& packet,
        const RenderObject& object
    ) {
        if (object.model == nullptr) {
            return false;
        }

        const bool texturedRenderType =
            packet.renderType == 2 ||
            packet.renderType == 3;

        const bool hasMapping =
            packet.textureUVMappingIndex >= 0 &&
            packet.textureUVMappingIndex <
                static_cast<int>(object.model->textureUVMappings.size());

        const bool hasTexture =
            object.model->textures.find(packet.color) !=
            object.model->textures.end();

        return
            texturedRenderType &&
            hasMapping &&
            hasTexture;
    }

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

        if (isTexturedPacket(packet, object)) {
            const eld::model::TextureUVMapping& mapping =
                object.model->textureUVMappings[packet.textureUVMappingIndex];

            const eld::texture::TextureAsset& texture =
                object.model->textures.at(packet.color);

            const eld::render::Vec3& faceA =
                mesh.vertices[packet.a].world;

            const eld::render::Vec3& faceB =
                mesh.vertices[packet.b].world;

            const eld::render::Vec3& faceC =
                mesh.vertices[packet.c].world;

            const eld::render::Vec3& textureOrigin =
                mesh.vertices[mapping.originVertex].world;

            const eld::render::Vec3& textureU =
                mesh.vertices[mapping.uVertex].world;

            const eld::render::Vec3& textureV =
                mesh.vertices[mapping.vVertex].world;

            rasterizer_.drawTexturedTriangle(
                framebuffer_,
                a,
                b,
                c,
                faceA,
                faceB,
                faceC,
                textureOrigin,
                textureU,
                textureV,
                texture
            );

            continue;
        }

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

void SoftwareRenderBackend::drawRect(int x, int y, int width, int height, ColorPixel color) {
    // Clamp to framebuffer bounds
    int x1 = std::max(0, x);
    int y1 = std::max(0, y);
    int x2 = std::min(width_, x + width);
    int y2 = std::min(height_, y + height);

    if (x1 >= x2 || y1 >= y2) return;

    for (int py = y1; py < y2; ++py) {
        for (int px = x1; px < x2; ++px) {
            framebuffer_.color().at(px, py) = color;
        }
    }
}

void SoftwareRenderBackend::drawRectOutline(int x, int y, int width, int height, ColorPixel color, int thickness) {
    // Draw top
    drawRect(x, y, width, thickness, color);
    // Draw bottom
    drawRect(x, y + height - thickness, width, thickness, color);
    // Draw left
    drawRect(x, y, thickness, height, color);
    // Draw right
    drawRect(x + width - thickness, y, thickness, height, color);
}

} // namespace eld::render
