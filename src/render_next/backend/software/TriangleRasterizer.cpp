#include "TriangleRasterizer.h"

#include <algorithm>
#include <cmath>

namespace rf::render_next {

namespace {

void drawPixel(
    Framebuffer& framebuffer,
    int x,
    int y,
    ColorPixel color
) {
    if (
        x < 0 ||
        y < 0 ||
        x >= framebuffer.color().width() ||
        y >= framebuffer.color().height()
    ) {
        return;
    }

    framebuffer.color().at(x, y) = color;
}

float edgeFunction(
    float ax,
    float ay,
    float bx,
    float by,
    float px,
    float py
) {
    return
        (px - ax) * (by - ay) -
        (py - ay) * (bx - ax);
}

}

void TriangleRasterizer::drawSolidTriangle(
    Framebuffer& framebuffer,

    const rf::render::ScreenPoint& a,
    const rf::render::ScreenPoint& b,
    const rf::render::ScreenPoint& c,

    ColorPixel color
) const {
    const int minX =
        static_cast<int>(
            std::floor(
                std::min({ a.x, b.x, c.x })
            )
        );

    const int maxX =
        static_cast<int>(
            std::ceil(
                std::max({ a.x, b.x, c.x })
            )
        );

    const int minY =
        static_cast<int>(
            std::floor(
                std::min({ a.y, b.y, c.y })
            )
        );

    const int maxY =
        static_cast<int>(
            std::ceil(
                std::max({ a.y, b.y, c.y })
            )
        );

    const float area =
        edgeFunction(
            a.x,
            a.y,
            b.x,
            b.y,
            c.x,
            c.y
        );

    if (area == 0.0f) {
        return;
    }

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            const float px =
                static_cast<float>(x) + 0.5f;

            const float py =
                static_cast<float>(y) + 0.5f;

            const float w0 =
                edgeFunction(
                    b.x,
                    b.y,
                    c.x,
                    c.y,
                    px,
                    py
                );

            const float w1 =
                edgeFunction(
                    c.x,
                    c.y,
                    a.x,
                    a.y,
                    px,
                    py
                );

            const float w2 =
                edgeFunction(
                    a.x,
                    a.y,
                    b.x,
                    b.y,
                    px,
                    py
                );

            const bool inside =
                (
                    w0 >= 0.0f &&
                    w1 >= 0.0f &&
                    w2 >= 0.0f
                ) ||
                (
                    w0 <= 0.0f &&
                    w1 <= 0.0f &&
                    w2 <= 0.0f
                );

            if (!inside) {
                continue;
            }

            const float normalizedW0 = w0 / area;
            const float normalizedW1 = w1 / area;
            const float normalizedW2 = w2 / area;

            const float depth =
                normalizedW0 * a.z +
                normalizedW1 * b.z +
                normalizedW2 * c.z;

            if (
                !framebuffer.depth().testAndWrite(
                    x,
                    y,
                    depth
                )
            ) {
                continue;
            }

            drawPixel(
                framebuffer,
                x,
                y,
                color
            );
        }
    }
}

}
