#pragma once

#include <cstdint>
#include <vector>

#include "../TextureAsset.h"
#include "../TextureFile.h"

namespace eld::texture {

class TextureCanvasDecoder {
public:
    TextureCanvasDecoder(
        const TextureFile& file,
        const std::vector<uint8_t>& indexedPixels
    );

    std::vector<RgbaColor> decode() const;

private:
    RgbaColor resolveColor(
        uint8_t paletteIndex
    ) const;

private:
    const TextureFile& file_;
    const std::vector<uint8_t>& indexedPixels_;
};

}
