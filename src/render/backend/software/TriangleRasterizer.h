#pragma once

#include "Framebuffer.h"
#include "SoftwareMeshProjector.h"
#include "graphics/model/RenderModel.h"
#include "graphics/texture/GraphicsTexture.h"

namespace eld::render {

class TriangleRasterizer {
public:
    void drawTriangle(
        Framebuffer& framebuffer,
        const SoftwareProjectedVertex& a,
        const SoftwareProjectedVertex& b,
        const SoftwareProjectedVertex& c,
        const eld::graphics::RenderMaterial& material,
        const eld::graphics::GraphicsTexture* texture,
        float depthBias = 0.0f
    ) const;
};

}
