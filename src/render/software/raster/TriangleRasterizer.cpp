#include "TriangleRasterizer.h"

#include <algorithm>
#include <cmath>

namespace rf::render {

namespace {

float edge(
    const ScreenPoint& a,
    const ScreenPoint& b,
    float x,
    float y
) {
    return
        (x - a.x) * (b.y - a.y) -
        (y - a.y) * (b.x - a.x);
}

const rf::texture::RgbaColor* sampleTexture(
    const rf::texture::TextureAsset& texture,
    float u,
    float v
) {
    const int width =
        texture.metadata.canvasWidth;

    const int height =
        texture.metadata.canvasHeight;

    if (
        width <= 0 ||
        height <= 0 ||
        texture.pixels.empty()
    ) {
        return nullptr;
    }

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
                width - 1
            )
        );

    int ty =
        static_cast<int>(
            v * static_cast<float>(
                height - 1
            )
        );

    int pixelIndex =
        ty * width + tx;

    if (
        pixelIndex < 0 ||
        pixelIndex >= static_cast<int>(
            texture.pixels.size()
        )
    ) {
        return nullptr;
    }

    return &texture.pixels[pixelIndex];
}

}

void fillTexturedTriangle(
    SDL_Renderer* renderer,
    DepthBuffer& depthBuffer,

    const ScreenPoint& a,
    const ScreenPoint& b,
    const ScreenPoint& c,

    const TextureMappingPoint& faceA,
    const TextureMappingPoint& faceB,
    const TextureMappingPoint& faceC,

    const TextureMappingPoint& textureOrigin,
    const TextureMappingPoint& textureU,
    const TextureMappingPoint& textureV,

    const rf::texture::TextureAsset& texture
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

    float basisUX =
        textureU.x - textureOrigin.x;

    float basisUY =
        textureU.y - textureOrigin.y;

    float basisUZ =
        textureU.z - textureOrigin.z;

    float basisVX =
        textureV.x - textureOrigin.x;

    float basisVY =
        textureV.y - textureOrigin.y;

    float basisVZ =
        textureV.z - textureOrigin.z;

    float uu =
        basisUX * basisUX +
        basisUY * basisUY +
        basisUZ * basisUZ;

    float uv =
        basisUX * basisVX +
        basisUY * basisVY +
        basisUZ * basisVZ;

    float vv =
        basisVX * basisVX +
        basisVY * basisVY +
        basisVZ * basisVZ;

    float determinant =
        uu * vv - uv * uv;

    if (std::abs(determinant) < 0.0001f) {
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

            float w0 = edge(b, c, px, py) / area;
            float w1 = edge(c, a, px, py) / area;
            float w2 = edge(a, b, px, py) / area;

            if (
                w0 < 0.0f ||
                w1 < 0.0f ||
                w2 < 0.0f
            ) {
                continue;
            }

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

            TextureMappingPoint point {
                faceA.x * w0 + faceB.x * w1 + faceC.x * w2,
                faceA.y * w0 + faceB.y * w1 + faceC.y * w2,
                faceA.z * w0 + faceB.z * w1 + faceC.z * w2
            };

            float relativeX =
                point.x - textureOrigin.x;

            float relativeY =
                point.y - textureOrigin.y;

            float relativeZ =
                point.z - textureOrigin.z;

            float projectedU =
                relativeX * basisUX +
                relativeY * basisUY +
                relativeZ * basisUZ;

            float projectedV =
                relativeX * basisVX +
                relativeY * basisVY +
                relativeZ * basisVZ;

            float u =
                (projectedU * vv - projectedV * uv) /
                determinant;

            float v =
                (projectedV * uu - projectedU * uv) /
                determinant;

            const rf::texture::RgbaColor* pixel =
                sampleTexture(
                    texture,
                    u,
                    v
                );

            if (pixel == nullptr) {
                continue;
            }

            if (
                pixel->a == 0 ||
                (
                    pixel->r == 0 &&
                    pixel->g == 0 &&
                    pixel->b == 0
                )
            ) {
                continue;
            }

            SDL_SetRenderDrawColor(
                renderer,
                pixel->r,
                pixel->g,
                pixel->b,
                pixel->a
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

            float w0 = edge(b, c, px, py) / area;
            float w1 = edge(c, a, px, py) / area;
            float w2 = edge(a, b, px, py) / area;

            if (
                w0 < 0.0f ||
                w1 < 0.0f ||
                w2 < 0.0f
            ) {
                continue;
            }

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
