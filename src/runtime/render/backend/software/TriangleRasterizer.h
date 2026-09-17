#pragma once

#include "Framebuffer.h"
#include "SoftwareMeshProjector.h"
#include "render/texture/TextureResource.h"
#include "render/model/ModelResource.h"

namespace eld::render {

class TriangleRasterizer {
public:
    void drawTriangle(
        Framebuffer& framebuffer,
        const SoftwareProjectedVertex& a,
        const SoftwareProjectedVertex& b,
        const SoftwareProjectedVertex& c,
        const eld::render::RenderMaterial& material,
        const eld::render::TextureResource* texture,
        float depthBias = 0.0f
    ) const;
};

}
