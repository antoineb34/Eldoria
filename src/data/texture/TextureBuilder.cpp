#include "TextureBuilder.h"

#include "decoder/TextureCanvasDecoder.h"
#include "decoder/TexturePixelDecoder.h"

namespace eld::texture {

    TextureAsset TextureBuilder::build(
        const TextureFile& file
    ) const {
        TexturePixelDecoder pixelDecoder(file);

        std::vector<uint8_t> indexedPixels =
            pixelDecoder.decode();

        TextureCanvasDecoder canvasDecoder(
            file,
            indexedPixels
        );

        return {
            file.metadata,
            file.palette,
            canvasDecoder.decode()
        };
    }

}
