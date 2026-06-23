#include "TriangleRasterizer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "TextureSampler.h"

namespace eld::render {

namespace {

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

std::uint8_t toByte(
    float value
) {
    return static_cast<std::uint8_t>(
        std::clamp(
            value * 255.0f,
            0.0f,
            255.0f
        )
    );
}

void drawPixel(
    Framebuffer& framebuffer,
    int x,
    int y,
    ColorPixel source
) {
    if (source.a == 0) {
        return;
    }

    if (source.a == 255) {
        framebuffer.color().at(
            x,
            y
        ) = source;

        return;
    }

    ColorPixel& destination =
        framebuffer.color().at(
            x,
            y
        );

    const float alpha =
        static_cast<float>(
            source.a
        ) /
        255.0f;

    destination.r =
        static_cast<std::uint8_t>(
            static_cast<float>(
                source.r
            ) *
                alpha +
            static_cast<float>(
                destination.r
            ) *
                (1.0f - alpha)
        );

    destination.g =
        static_cast<std::uint8_t>(
            static_cast<float>(
                source.g
            ) *
                alpha +
            static_cast<float>(
                destination.g
            ) *
                (1.0f - alpha)
        );

    destination.b =
        static_cast<std::uint8_t>(
            static_cast<float>(
                source.b
            ) *
                alpha +
            static_cast<float>(
                destination.b
            ) *
                (1.0f - alpha)
        );

    destination.a = 255;
}

}

void TriangleRasterizer::drawTriangle(
    Framebuffer& framebuffer,
    const SoftwareProjectedVertex& a,
    const SoftwareProjectedVertex& b,
    const SoftwareProjectedVertex& c,
    const Material& material
) const {
    if (
        !a.valid ||
        !b.valid ||
        !c.valid
    ) {
        return;
    }

    constexpr float MinimumDepth =
        0.0001f;

    if (
        std::abs(a.screen.z) <
            MinimumDepth ||
        std::abs(b.screen.z) <
            MinimumDepth ||
        std::abs(c.screen.z) <
            MinimumDepth
    ) {
        return;
    }

    const float area =
        edgeFunction(
            a.screen.x,
            a.screen.y,
            b.screen.x,
            b.screen.y,
            c.screen.x,
            c.screen.y
        );

    if (
        std::abs(area) <
        0.0001f
    ) {
        return;
    }

    const int framebufferWidth =
        framebuffer.color().width();

    const int framebufferHeight =
        framebuffer.color().height();

    if (
        framebufferWidth <= 0 ||
        framebufferHeight <= 0
    ) {
        return;
    }

    const int minX =
        std::max(
            0,
            static_cast<int>(
                std::floor(
                    std::min({
                        a.screen.x,
                        b.screen.x,
                        c.screen.x
                    })
                )
            )
        );

    const int maxX =
        std::min(
            framebufferWidth - 1,
            static_cast<int>(
                std::ceil(
                    std::max({
                        a.screen.x,
                        b.screen.x,
                        c.screen.x
                    })
                )
            )
        );

    const int minY =
        std::max(
            0,
            static_cast<int>(
                std::floor(
                    std::min({
                        a.screen.y,
                        b.screen.y,
                        c.screen.y
                    })
                )
            )
        );

    const int maxY =
        std::min(
            framebufferHeight - 1,
            static_cast<int>(
                std::ceil(
                    std::max({
                        a.screen.y,
                        b.screen.y,
                        c.screen.y
                    })
                )
            )
        );

    if (
        minX > maxX ||
        minY > maxY
    ) {
        return;
    }

    const float inverseDepthA =
        1.0f / a.screen.z;

    const float inverseDepthB =
        1.0f / b.screen.z;

    const float inverseDepthC =
        1.0f / c.screen.z;

    TextureSampler sampler;

    for (
        int y = minY;
        y <= maxY;
        y++
    ) {
        for (
            int x = minX;
            x <= maxX;
            x++
        ) {
            const float pixelX =
                static_cast<float>(x) +
                0.5f;

            const float pixelY =
                static_cast<float>(y) +
                0.5f;

            const float weightA =
                edgeFunction(
                    b.screen.x,
                    b.screen.y,
                    c.screen.x,
                    c.screen.y,
                    pixelX,
                    pixelY
                ) /
                area;

            const float weightB =
                edgeFunction(
                    c.screen.x,
                    c.screen.y,
                    a.screen.x,
                    a.screen.y,
                    pixelX,
                    pixelY
                ) /
                area;

            const float weightC =
                edgeFunction(
                    a.screen.x,
                    a.screen.y,
                    b.screen.x,
                    b.screen.y,
                    pixelX,
                    pixelY
                ) /
                area;

            if (
                weightA < 0.0f ||
                weightB < 0.0f ||
                weightC < 0.0f
            ) {
                continue;
            }

            const float inverseDepth =
                weightA * inverseDepthA +
                weightB * inverseDepthB +
                weightC * inverseDepthC;

            if (
                std::abs(inverseDepth) <
                MinimumDepth
            ) {
                continue;
            }

            const float correctedA =
                weightA *
                inverseDepthA /
                inverseDepth;

            const float correctedB =
                weightB *
                inverseDepthB /
                inverseDepth;

            const float correctedC =
                weightC *
                inverseDepthC /
                inverseDepth;

            const float depth =
                1.0f /
                inverseDepth;

            const Vec2 uv{
                a.uv.x * correctedA +
                    b.uv.x * correctedB +
                    c.uv.x * correctedC,
                a.uv.y * correctedA +
                    b.uv.y * correctedB +
                    c.uv.y * correctedC
            };

            const Vec4 vertexColor{
                a.color.x * correctedA +
                    b.color.x * correctedB +
                    c.color.x * correctedC,
                a.color.y * correctedA +
                    b.color.y * correctedB +
                    c.color.y * correctedC,
                a.color.z * correctedA +
                    b.color.z * correctedB +
                    c.color.z * correctedC,
                a.color.w * correctedA +
                    b.color.w * correctedB +
                    c.color.w * correctedC
            };

            float textureRed = 1.0f;
            float textureGreen = 1.0f;
            float textureBlue = 1.0f;
            float textureAlpha = 1.0f;

            if (material.textured()) {
                const auto sampled =
                    sampler.sample(
                        *material.albedoTexture,
                        uv.x,
                        uv.y,
                        material.sampler
                    );

                textureRed =
                    static_cast<float>(
                        sampled.red
                    ) /
                    255.0f;

                textureGreen =
                    static_cast<float>(
                        sampled.green
                    ) /
                    255.0f;

                textureBlue =
                    static_cast<float>(
                        sampled.blue
                    ) /
                    255.0f;

                textureAlpha =
                    static_cast<float>(
                        sampled.alpha
                    ) /
                    255.0f;
            }

            const ColorPixel color{
                toByte(
                    vertexColor.x *
                    material.baseColor.x *
                    textureRed
                ),
                toByte(
                    vertexColor.y *
                    material.baseColor.y *
                    textureGreen
                ),
                toByte(
                    vertexColor.z *
                    material.baseColor.z *
                    textureBlue
                ),
                toByte(
                    vertexColor.w *
                    material.baseColor.w *
                    textureAlpha
                )
            };

            if (color.a == 0) {
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
                color
            );
        }
    }
}

}
