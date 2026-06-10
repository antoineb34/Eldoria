#pragma once

#include "../../../render/camera/Projection.h"
#include "Framebuffer.h"
#include "texture/TextureAsset.h"
#include "../../material/TextureSampler.h"

namespace eld::render {

class TriangleRasterizer {
public:
    void drawSolidTriangle(
        Framebuffer& framebuffer,

        const eld::render::ScreenPoint& a,
        const eld::render::ScreenPoint& b,
        const eld::render::ScreenPoint& c,

        ColorPixel color
    ) const;

    void drawTexturedTriangle(
        Framebuffer& framebuffer,

        const eld::render::ScreenPoint& a,
        const eld::render::ScreenPoint& b,
        const eld::render::ScreenPoint& c,

        const eld::render::Vec3& faceA,
        const eld::render::Vec3& faceB,
        const eld::render::Vec3& faceC,

        const eld::render::Vec3& textureOrigin,
        const eld::render::Vec3& textureU,
        const eld::render::Vec3& textureV,

        const eld::texture::TextureAsset& texture
    ) const;
};

}
