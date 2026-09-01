#include "views/font/FontView.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace eld::elforge {

eld::image::Image FontView::build(
    const eld::font::Font& font
) const {
    constexpr std::size_t Columns = 16;
    constexpr std::size_t Rows = 16;
    constexpr std::size_t Padding = 2;

    std::size_t maximumWidth = 1;
    std::size_t maximumHeight = 1;

    for (const eld::font::Glyph& glyph : font.glyphs) {
        maximumWidth =
            std::max(
                maximumWidth,
                static_cast<std::size_t>(
                    glyph.offsetX
                ) +
                glyph.width
            );

        maximumHeight =
            std::max(
                maximumHeight,
                static_cast<std::size_t>(
                    glyph.offsetY
                ) +
                glyph.height
            );
    }

    const std::size_t cellWidth =
        maximumWidth +
        Padding * 2;

    const std::size_t cellHeight =
        std::max(
            maximumHeight,
            static_cast<std::size_t>(
                font.lineHeight
            )
        ) +
        Padding * 2;

    const std::size_t atlasWidth =
        Columns *
        cellWidth;

    const std::size_t atlasHeight =
        Rows *
        cellHeight;

    if (
        atlasWidth >
            std::numeric_limits<std::uint16_t>::max() ||
        atlasHeight >
            std::numeric_limits<std::uint16_t>::max()
    ) {
        throw std::runtime_error(
            "Font view is too large"
        );
    }

    eld::image::Image image;

    image.width =
        static_cast<std::uint16_t>(
            atlasWidth
        );

    image.height =
        static_cast<std::uint16_t>(
            atlasHeight
        );

    image.pixels.resize(
        atlasWidth *
        atlasHeight
    );

    std::fill(
        image.pixels.begin(),
        image.pixels.end(),
        eld::image::RgbaPixel{
            24,
            24,
            18,
            255
        }
    );

    for (
        std::size_t glyphIndex = 0;
        glyphIndex < font.glyphs.size();
        glyphIndex++
    ) {
        const eld::font::Glyph& glyph =
            font.glyphs[glyphIndex];

        const std::size_t expectedPixelCount =
            static_cast<std::size_t>(
                glyph.width
            ) *
            glyph.height;

        if (
            glyph.alpha.size() !=
            expectedPixelCount
        ) {
            throw std::runtime_error(
                "Font glyph alpha size is invalid"
            );
        }

        const std::size_t cellX =
            glyphIndex %
            Columns;

        const std::size_t cellY =
            glyphIndex /
            Columns;

        if (cellY >= Rows) {
            break;
        }

        for (
            std::size_t y = 0;
            y < glyph.height;
            y++
        ) {
            for (
                std::size_t x = 0;
                x < glyph.width;
                x++
            ) {
                const std::size_t source =
                    y *
                    glyph.width +
                    x;

                const std::uint8_t alpha =
                    glyph.alpha[source];

                if (alpha == 0) {
                    continue;
                }

                const std::size_t destinationX =
                    cellX *
                        cellWidth +
                    Padding +
                    glyph.offsetX +
                    x;

                const std::size_t destinationY =
                    cellY *
                        cellHeight +
                    Padding +
                    glyph.offsetY +
                    y;

                const std::size_t destination =
                    destinationY *
                        atlasWidth +
                    destinationX;

                image.pixels[destination] =
                    eld::image::RgbaPixel{
                        255,
                        255,
                        0,
                        alpha
                    };
            }
        }
    }

    return image;
}

}
