#include "FloorPreviewBuilder.h"

#include <cstddef>
#include <cstdint>

namespace eld::elforge {

namespace {

eld::image::RgbaPixel makeColorPixel(
    std::uint32_t rgb
) {
    return eld::image::RgbaPixel{
        static_cast<std::uint8_t>(
            (rgb >> 16) & 0xFF
        ),
        static_cast<std::uint8_t>(
            (rgb >> 8) & 0xFF
        ),
        static_cast<std::uint8_t>(
            rgb & 0xFF
        ),
        255
    };
}

}

eld::image::Image FloorPreviewBuilder::build(
    const eld::definition::FloorDefinition& floor
) const {
    constexpr std::uint16_t Size = 256;

    eld::image::Image image;

    image.width = Size;
    image.height = Size;
    image.pixels.resize(
        static_cast<std::size_t>(Size) *
        Size
    );

    const eld::image::RgbaPixel primary =
        makeColorPixel(
            floor.rgb.value_or(0)
        );

    const eld::image::RgbaPixel secondary =
        makeColorPixel(
            floor.secondaryRgb.value_or(
                floor.rgb.value_or(0)
            )
        );

    for (
        std::size_t y = 0;
        y < Size;
        y++
    ) {
        for (
            std::size_t x = 0;
            x < Size;
            x++
        ) {
            image.pixels[
                y *
                    Size +
                x
            ] =
                x < Size / 2
                    ? primary
                    : secondary;
        }
    }

    return image;
}

}
