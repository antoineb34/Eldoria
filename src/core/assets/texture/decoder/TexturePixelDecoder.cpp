#include "TexturePixelDecoder.h"

namespace rf::texture {

TexturePixelDecoder::TexturePixelDecoder(
    const TextureFile& file
)
    : file_(file)
{
}

std::vector<uint8_t> TexturePixelDecoder::decode() const {
    if (file_.metadata.type == 1) {
        return decodeType1();
    }

    return decodeType0();
}

std::vector<uint8_t> TexturePixelDecoder::decodeType0() const {
    return file_.indexedPixels;
}

std::vector<uint8_t> TexturePixelDecoder::decodeType1() const {
    const int width =
        file_.metadata.width;

    const int height =
        file_.metadata.height;

    std::vector<uint8_t> pixels(
        width * height
    );

    int source = 0;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            pixels[y * width + x] =
                file_.indexedPixels[source++];
        }
    }

    return pixels;
}

}
