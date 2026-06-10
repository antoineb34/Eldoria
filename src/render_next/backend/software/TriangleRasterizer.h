#pragma once

#include "../../../render/software/camera/Projection.h"
#include "Framebuffer.h"
#include "texture/TextureAsset.h"
#include "../../material/TextureSampler.h"

namespace rf::render_next {

class TriangleRasterizer {
public:
    void drawSolidTriangle(
        Framebuffer& framebuffer,

        const rf::render::ScreenPoint& a,
        const rf::render::ScreenPoint& b,
        const rf::render::ScreenPoint& c,

        ColorPixel color
    ) const;

    void drawTexturedTriangle(
        Framebuffer& framebuffer,

        const rf::render::ScreenPoint& a,
        const rf::render::ScreenPoint& b,
        const rf::render::ScreenPoint& c,

        const rf::render::Vec3& faceA,
        const rf::render::Vec3& faceB,
        const rf::render::Vec3& faceC,

        const rf::render::Vec3& textureOrigin,
        const rf::render::Vec3& textureU,
        const rf::render::Vec3& textureV,

        const rf::texture::TextureAsset& texture
    ) const;
};

}
