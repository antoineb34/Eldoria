#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "TextureFile.h"

namespace eld::texture {

class TextureFileReader {
public:
    std::optional<TextureFile> read(
        int id,
        const std::vector<uint8_t>& indexData,
        const std::vector<uint8_t>& textureData
    ) const;

private:
    TexturePalette readPalette(
        const std::vector<uint8_t>& indexData,
        int indexOffset,
        int paletteCount
    ) const;

    TextureMetadata readMetadata(
        const std::vector<uint8_t>& indexData,
        int indexOffset,
        int paletteCount
    ) const;

    std::vector<uint8_t> readIndexedPixels(
        const std::vector<uint8_t>& textureData,
        const TextureMetadata& metadata
    ) const;
};

}
