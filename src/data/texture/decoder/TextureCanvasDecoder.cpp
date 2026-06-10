#include "TextureCanvasDecoder.h"

namespace eld::texture {

TextureCanvasDecoder::TextureCanvasDecoder(
    const TextureFile& file,
    const std::vector<uint8_t>& indexedPixels
)
    : file_(file),
      indexedPixels_(indexedPixels)
{
}

std::vector<RgbaColor> TextureCanvasDecoder::decode() const {
    const int canvasWidth =
        file_.metadata.canvasWidth;

    const int canvasHeight =
        file_.metadata.canvasHeight;

    const int imageWidth =
        file_.metadata.width;

    const int imageHeight =
        file_.metadata.height;

    std::vector<RgbaColor> pixels(
        canvasWidth * canvasHeight,
        { 0, 0, 0, 0 }
    );

    for (int y = 0; y < imageHeight; y++) {
        for (int x = 0; x < imageWidth; x++) {
            int sourceIndex =
                y * imageWidth + x;

            int destinationX =
                x + file_.metadata.xOffset;

            int destinationY =
                y + file_.metadata.yOffset;

            if (
                destinationX < 0 ||
                destinationY < 0 ||
                destinationX >= canvasWidth ||
                destinationY >= canvasHeight
            ) {
                continue;
            }

            int destinationIndex =
                destinationY * canvasWidth +
                destinationX;

            pixels[destinationIndex] =
                resolveColor(
                    indexedPixels_[sourceIndex]
                );
        }
    }

    return pixels;
}

RgbaColor TextureCanvasDecoder::resolveColor(
    uint8_t paletteIndex
) const {
    if (
        paletteIndex >= file_.palette.colors.size()
    ) {
        return {
            0,
            0,
            0,
            0
        };
    }

    const RgbColor& color =
        file_.palette.colors[paletteIndex];

    return {
        color.r,
        color.g,
        color.b,
        255
    };
}

}
