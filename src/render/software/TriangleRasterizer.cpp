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

void fillTexturedTriangle(
    SDL_Renderer* renderer,
    DepthBuffer& depthBuffer,
    const ScreenPoint& a,
    const ScreenPoint& b,
    const ScreenPoint& c,
    const rf::texture::DecodedTexture& texture
) {
    if (
        texture.width <= 0 ||
        texture.height <= 0 ||
        texture.pixels.empty()
    ) {
        return;
    }

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
                !depthBuffer.testAndSet(
                    x,
                    y,
                    depth
                )
            ) {
                continue;
            }

            float u =
                w1;

            float v =
                w2;

            u = std::clamp(
                u,
                0.0f,
                1.0f
            );

            v = std::clamp(
                v,
                0.0f,
                1.0f
            );

            int tx =
                static_cast<int>(
                    u * static_cast<float>(
                        texture.width - 1
                    )
                );

            int ty =
                static_cast<int>(
                    v * static_cast<float>(
                        texture.height - 1
                    )
                );

            int pixelIndex =
                (
                    ty * texture.width +
                    tx
                ) * 4;

            if (
                pixelIndex < 0 ||
                pixelIndex + 3 >=
                    static_cast<int>(
                        texture.pixels.size()
                    )
            ) {
                continue;
            }

            SDL_SetRenderDrawColor(
                renderer,
                texture.pixels[pixelIndex + 0],
                texture.pixels[pixelIndex + 1],
                texture.pixels[pixelIndex + 2],
                texture.pixels[pixelIndex + 3]
            );

            SDL_RenderPoint(
                renderer,
                static_cast<float>(x),
                static_cast<float>(y)
            );
        }
    }
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
