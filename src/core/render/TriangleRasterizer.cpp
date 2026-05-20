#include "TriangleRasterizer.h"

#include <algorithm>
#include <cmath>

namespace rf::render {

static float edge(
    const ScreenPoint& a,
    const ScreenPoint& b,
    float x,
    float y
) {
    return
        (x - a.x) * (b.y - a.y) -
        (y - a.y) * (b.x - a.x);
}

void fillTriangle(
    SDL_Renderer* renderer,
    DepthBuffer& depthBuffer,
    const ScreenPoint& a,
    const ScreenPoint& b,
    const ScreenPoint& c
) {
    float minX =
        std::floor(
            std::min({ a.x, b.x, c.x })
        );

    float maxX =
        std::ceil(
            std::max({ a.x, b.x, c.x })
        );

    float minY =
        std::floor(
            std::min({ a.y, b.y, c.y })
        );

    float maxY =
        std::ceil(
            std::max({ a.y, b.y, c.y })
        );

    float area =
        edge(a, b, c.x, c.y);

    if (std::abs(area) < 0.0001f) {
        return;
    }

    for (
        int y = static_cast<int>(minY);
        y <= static_cast<int>(maxY);
        y++
    ) {
        for (
            int x = static_cast<int>(minX);
            x <= static_cast<int>(maxX);
            x++
        ) {
            float px =
                static_cast<float>(x) + 0.5f;

            float py =
                static_cast<float>(y) + 0.5f;

            float w0 =
                edge(b, c, px, py);

            float w1 =
                edge(c, a, px, py);

            float w2 =
                edge(a, b, px, py);

            bool inside =
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

            w0 /= area;
            w1 /= area;
            w2 /= area;

            float depth =
                a.z * w0 +
                b.z * w1 +
                c.z * w2;

            if (
                depthBuffer.testAndSet(
                    x,
                    y,
                    depth
                )
            ) {
                SDL_RenderPoint(
                    renderer,
                    static_cast<float>(x),
                    static_cast<float>(y)
                );
            }
        }
    }
}

}
