#pragma once

#include "../../../render/software/camera/Projection.h"
#include "Framebuffer.h"

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
};

}
