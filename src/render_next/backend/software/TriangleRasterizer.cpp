#include "TriangleRasterizer.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "../../material/TextureSampler.h"

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

        if (color.a >= 255) {
            framebuffer.color().at(x, y) = color;
            return;
        }

        ColorPixel& dst =
            framebuffer.color().at(x, y);

        float alpha =
            static_cast<float>(color.a) / 255.0f;

        dst.r = static_cast<uint8_t>(
            static_cast<float>(color.r) * alpha +
            static_cast<float>(dst.r) * (1.0f - alpha)
        );

        dst.g = static_cast<uint8_t>(
            static_cast<float>(color.g) * alpha +
            static_cast<float>(dst.g) * (1.0f - alpha)
        );

        dst.b = static_cast<uint8_t>(
            static_cast<float>(color.b) * alpha +
            static_cast<float>(dst.b) * (1.0f - alpha)
        );

        dst.a = 255;
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

void TriangleRasterizer::drawTexturedTriangle(
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
) const {
    TextureSampler sampler;

    const float minX = std::floor(std::min({ a.x, b.x, c.x }));
    const float maxX = std::ceil(std::max({ a.x, b.x, c.x }));
    const float minY = std::floor(std::min({ a.y, b.y, c.y }));
    const float maxY = std::ceil(std::max({ a.y, b.y, c.y }));

    const float area =
        edgeFunction(a.x, a.y, b.x, b.y, c.x, c.y);

    if (std::abs(area) < 0.0001f) {
        return;
    }

    const rf::render::Vec3 basisU {
        textureU.x - textureOrigin.x,
        textureU.y - textureOrigin.y,
        textureU.z - textureOrigin.z
    };

    const rf::render::Vec3 basisV {
        textureV.x - textureOrigin.x,
        textureV.y - textureOrigin.y,
        textureV.z - textureOrigin.z
    };

    const float uu =
        basisU.x * basisU.x +
        basisU.y * basisU.y +
        basisU.z * basisU.z;

    const float uv =
        basisU.x * basisV.x +
        basisU.y * basisV.y +
        basisU.z * basisV.z;

    const float vv =
        basisV.x * basisV.x +
        basisV.y * basisV.y +
        basisV.z * basisV.z;

    const float determinant =
        uu * vv - uv * uv;

    if (std::abs(determinant) < 0.0001f) {
        return;
    }

    float minUSeen = 999999.0f;
    float maxUSeen = -999999.0f;
    float minVSeen = 999999.0f;
    float maxVSeen = -999999.0f;

    int insidePixels = 0;
    int sampledPixels = 0;
    int outOfRangePixels = 0;

    for (int y = static_cast<int>(minY); y <= static_cast<int>(maxY); y++) {
        for (int x = static_cast<int>(minX); x <= static_cast<int>(maxX); x++) {
            const float px = static_cast<float>(x) + 0.5f;
            const float py = static_cast<float>(y) + 0.5f;

            const float w0 =
                edgeFunction(b.x, b.y, c.x, c.y, px, py);

            const float w1 =
                edgeFunction(c.x, c.y, a.x, a.y, px, py);

            const float w2 =
                edgeFunction(a.x, a.y, b.x, b.y, px, py);

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

            insidePixels++;

            const float normalizedW0 = w0 / area;
            const float normalizedW1 = w1 / area;
            const float normalizedW2 = w2 / area;

            const float depth =
                normalizedW0 * a.z +
                normalizedW1 * b.z +
                normalizedW2 * c.z;

            const rf::render::Vec3 point {
                faceA.x * normalizedW0 + faceB.x * normalizedW1 + faceC.x * normalizedW2,
                faceA.y * normalizedW0 + faceB.y * normalizedW1 + faceC.y * normalizedW2,
                faceA.z * normalizedW0 + faceB.z * normalizedW1 + faceC.z * normalizedW2
            };

            const rf::render::Vec3 relative {
                point.x - textureOrigin.x,
                point.y - textureOrigin.y,
                point.z - textureOrigin.z
            };

            const float projectedU =
                relative.x * basisU.x +
                relative.y * basisU.y +
                relative.z * basisU.z;

            const float projectedV =
                relative.x * basisV.x +
                relative.y * basisV.y +
                relative.z * basisV.z;

            const float u =
                (projectedU * vv - projectedV * uv) /
                determinant;

            const float v =
                (projectedV * uu - projectedU * uv) /
                determinant;

            minUSeen = std::min(minUSeen, u);
            maxUSeen = std::max(maxUSeen, u);
            minVSeen = std::min(minVSeen, v);
            maxVSeen = std::max(maxVSeen, v);

            if (
                u < 0.0f ||
                u > 1.0f ||
                v < 0.0f ||
                v > 1.0f
            ) {
                outOfRangePixels++;
            }

            const rf::texture::RgbaColor* pixel =
                sampler.sample(
                    texture,
                    u,
                    v
                );

            if (pixel == nullptr) {
                continue;
            }

            sampledPixels++;

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
                {
                    pixel->r,
                    pixel->g,
                    pixel->b,
                    pixel->a
                }
            );
        }
    }

    if (
        insidePixels > 0 &&
        outOfRangePixels > insidePixels / 10
    ) {
        std::cout
            << "TRI UV RANGE "
            << "u=" << minUSeen << ".." << maxUSeen
            << " v=" << minVSeen << ".." << maxVSeen
            << " inside=" << insidePixels
            << " sampled=" << sampledPixels
            << " outOfRange=" << outOfRangePixels
            << std::endl;
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
