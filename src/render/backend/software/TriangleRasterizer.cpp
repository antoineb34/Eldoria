#include "TriangleRasterizer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "TextureSampler.h"

namespace eld::render {

namespace {

constexpr float Epsilon = 0.0001f;

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

void blendPixel(
    Framebuffer& framebuffer,
    int x,
    int y,
    const ColorPixel& source
) {
    if (source.alpha == 0) {
        return;
    }

    ColorPixel& destination =
        framebuffer.color().at(
            static_cast<std::uint32_t>(x),
            static_cast<std::uint32_t>(y)
        );

    if (source.alpha == 255) {
        destination = source;
        return;
    }

    const float alpha =
        static_cast<float>(source.alpha) /
        255.0f;

    destination.red =
        static_cast<std::uint8_t>(
            source.red * alpha +
            destination.red * (1.0f - alpha)
        );

    destination.green =
        static_cast<std::uint8_t>(
            source.green * alpha +
            destination.green * (1.0f - alpha)
        );

    destination.blue =
        static_cast<std::uint8_t>(
            source.blue * alpha +
            destination.blue * (1.0f - alpha)
        );

    destination.alpha = 255;
}

}

void TriangleRasterizer::drawTriangle(
    Framebuffer& framebuffer,
    const SoftwareProjectedVertex& a,
    const SoftwareProjectedVertex& b,
    const SoftwareProjectedVertex& c,
    const eld::graphics::RenderMaterial& material,
    const eld::graphics::GraphicsTexture* texture,
    float depthBias
) const {
    if (!a.valid || !b.valid || !c.valid) {
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

    if (std::abs(area) < Epsilon) {
        return;
    }

    const int width =
        static_cast<int>(
            framebuffer.color().width()
        );

    const int height =
        static_cast<int>(
            framebuffer.color().height()
        );

    if (width <= 0 || height <= 0) {
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
            width - 1,
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
            height - 1,
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

    if (minX > maxX || minY > maxY) {
        return;
    }

    const float inverseDepthA =
        1.0f / a.screen.depth;

    const float inverseDepthB =
        1.0f / b.screen.depth;

    const float inverseDepthC =
        1.0f / c.screen.depth;

    TextureSampler sampler;

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            const float pixelX =
                static_cast<float>(x) + 0.5f;

            const float pixelY =
                static_cast<float>(y) + 0.5f;

            const float weightA =
                edgeFunction(
                    b.screen.x,
                    b.screen.y,
                    c.screen.x,
                    c.screen.y,
                    pixelX,
                    pixelY
                ) / area;

            const float weightB =
                edgeFunction(
                    c.screen.x,
                    c.screen.y,
                    a.screen.x,
                    a.screen.y,
                    pixelX,
                    pixelY
                ) / area;

            const float weightC =
                edgeFunction(
                    a.screen.x,
                    a.screen.y,
                    b.screen.x,
                    b.screen.y,
                    pixelX,
                    pixelY
                ) / area;

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

            if (std::abs(inverseDepth) < Epsilon) {
                continue;
            }

            const float correctedA =
                weightA * inverseDepthA /
                inverseDepth;

            const float correctedB =
                weightB * inverseDepthB /
                inverseDepth;

            const float correctedC =
                weightC * inverseDepthC /
                inverseDepth;

            const float depth =
                1.0f / inverseDepth +
                depthBias;

            const eld::math::Vec2 uv{
                a.uv.x * correctedA +
                    b.uv.x * correctedB +
                    c.uv.x * correctedC,

                a.uv.y * correctedA +
                    b.uv.y * correctedB +
                    c.uv.y * correctedC
            };

            const eld::math::Vec4 vertexColor{
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

            ColorPixel sampled{
                255,
                255,
                255,
                255
            };

            if (texture != nullptr) {
                sampled =
                    sampler.sample(
                        *texture,
                        uv.x,
                        uv.y,
                        material.sampler
                    );
            }

            ColorPixel color{
                toByte(
                    vertexColor.x *
                    material.baseColor.x *
                    sampled.red / 255.0f
                ),

                toByte(
                    vertexColor.y *
                    material.baseColor.y *
                    sampled.green / 255.0f
                ),

                toByte(
                    vertexColor.z *
                    material.baseColor.z *
                    sampled.blue / 255.0f
                ),

                toByte(
                    vertexColor.w *
                    material.baseColor.w *
                    sampled.alpha / 255.0f
                )
            };

            if (
                material.alphaMode ==
                    eld::graphics::AlphaMode::Masked
            ) {
                if (color.alpha < 128) {
                    continue;
                }

                color.alpha = 255;
            }
            else if (
                material.alphaMode ==
                    eld::graphics::AlphaMode::Opaque
            ) {
                color.alpha = 255;
            }

            if (color.alpha == 0) {
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

            blendPixel(
                framebuffer,
                x,
                y,
                color
            );
        }
    }
}

}
