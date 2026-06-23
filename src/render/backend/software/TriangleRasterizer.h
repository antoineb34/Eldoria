#pragma once

#include "Framebuffer.h"
#include "SoftwareMeshProjector.h"

#include "../../material/Material.h"

namespace eld::render {

class TriangleRasterizer {
public:
    void drawTriangle(
        Framebuffer& framebuffer,
        const SoftwareProjectedVertex& a,
        const SoftwareProjectedVertex& b,
        const SoftwareProjectedVertex& c,
        const Material& material
    ) const;
};

}
