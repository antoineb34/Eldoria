#include "SoftwareRenderBackend.h"

#include <vector>

#include "../../../render/software/color/Color.h"

namespace rf::render_next {

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
        rf::render::RgbColor rgb =
            rf::render::rsColorToRgb(packet.color);

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
    int texturedPackets = 0;

    int type0 = 0;
    int type1 = 0;
    int type2 = 0;
    int type3 = 0;
    int otherTypes = 0;

    const auto rawVec = [](
        const rf::model::Vertex& vertex
    ) -> rf::render::Vec3 {
        return {
            vertex.x,
            vertex.y,
            vertex.z
        };
    };

    for (const RenderPacket& packet : queue.packets) {
        rf::render::ScreenPoint a =
            mesh.vertices[packet.a].screen;

        rf::render::ScreenPoint b =
            mesh.vertices[packet.b].screen;

        rf::render::ScreenPoint c =
            mesh.vertices[packet.c].screen;

        switch (packet.renderType) {
            case 0: type0++; break;
            case 1: type1++; break;
            case 2: type2++; break;
            case 3: type3++; break;
            default: otherTypes++; break;
        }

        if (isTexturedPacket(packet, object)) {
            texturedPackets++;

            const rf::model::TextureUVMapping& mapping =
                object.model->textureUVMappings[packet.textureUVMappingIndex];

            SDL_Log(
                "TEXTURED face=%d color=%d texPtr=%d uvMap=%d renderType=%d priority=%d "
                "verts=(%d,%d,%d) mapping=(%d,%d,%d)",
                packet.faceIndex,
                packet.color,
                packet.texturePointer,
                packet.textureUVMappingIndex,
                packet.renderType,
                packet.priority,
                packet.a,
                packet.b,
                packet.c,
                mapping.originVertex,
                mapping.uVertex,
                mapping.vVertex
            );

            const rf::texture::TextureAsset& texture =
                object.model->textures.at(packet.color);

            const rf::render::Vec3 faceA =
                rawVec(object.model->vertices[packet.a]);

            const rf::render::Vec3 faceB =
                rawVec(object.model->vertices[packet.b]);

            const rf::render::Vec3 faceC =
                rawVec(object.model->vertices[packet.c]);

            const rf::render::Vec3 textureOrigin =
                rawVec(object.model->vertices[mapping.originVertex]);

            const rf::render::Vec3 textureU =
                rawVec(object.model->vertices[mapping.uVertex]);

            const rf::render::Vec3 textureV =
                rawVec(object.model->vertices[mapping.vVertex]);

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

    SDL_Log(
        "render types: 0=%d 1=%d 2=%d 3=%d other=%d textured=%d total=%zu",
        type0,
        type1,
        type2,
        type3,
        otherTypes,
        texturedPackets,
        queue.packets.size()
    );
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
