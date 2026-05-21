#include "TextureDecoder.h"

#include <stdexcept>
#include <vector>

namespace rf::texture {

static std::uint16_t readU16(
    const std::vector<std::uint8_t>& data,
    std::size_t offset
) {
    if (offset + 1 >= data.size()) {
        throw std::runtime_error(
            "Texture read out of bounds"
        );
    }

    return static_cast<std::uint16_t>(
        (data[offset] << 8) |
        data[offset + 1]
    );
}

struct TextureDefinition {
    std::vector<RgbColor> palette;
    TextureMetadata metadata;
};

static TextureDefinition readTextureDefinition(
    const TextureIndex& index,
    std::size_t indexOffset
) {
    const auto& data =
        index.rawData;

    if (indexOffset + 5 >= data.size()) {
        throw std::runtime_error(
            "Texture index offset out of bounds"
        );
    }

    TextureDefinition def;

    // These are the texture canvas dimensions.
    // We do not need them yet, but reading them advances the cursor correctly.
    const int canvasWidth =
        readU16(data, indexOffset);

    const int canvasHeight =
        readU16(data, indexOffset + 2);

    (void)canvasWidth;
    (void)canvasHeight;

    const std::uint8_t paletteCount =
        data[indexOffset + 4];

    def.palette.resize(
        paletteCount
    );

    def.palette[0] = {
        0,
        0,
        0
    };

    std::size_t offset =
        indexOffset + 5;

    for (
        std::size_t i = 1;
        i < paletteCount;
        i++
    ) {
        if (offset + 2 >= data.size()) {
            throw std::runtime_error(
                "Texture palette read out of bounds"
            );
        }

        def.palette[i] = {
            data[offset],
            data[offset + 1],
            data[offset + 2]
        };

        offset += 3;
    }

    if (offset + 6 >= data.size()) {
        throw std::runtime_error(
            "Texture metadata read out of bounds"
        );
    }

    def.metadata.xOffset =
        data[offset];

    def.metadata.yOffset =
        data[offset + 1];

    def.metadata.width =
        readU16(data, offset + 2);

    def.metadata.height =
        readU16(data, offset + 4);

    def.metadata.type =
        data[offset + 6];

    return def;
}

DecodedTexture TextureDecoder::decode(
    const TextureIndex& index,
    const std::vector<std::uint8_t>& fileData,
    int textureId
) {
    (void)textureId;

    if (fileData.size() < 2) {
        throw std::runtime_error(
            "Texture file is too small"
        );
    }

    const std::uint16_t indexOffset =
        readU16(
            fileData,
            0
        );

    const TextureDefinition def =
        readTextureDefinition(
            index,
            indexOffset
        );

    const TextureMetadata& meta =
        def.metadata;

    constexpr std::size_t pixelOffset = 2;

    const std::size_t pixelCount =
        static_cast<std::size_t>(meta.width) *
        static_cast<std::size_t>(meta.height);

    if (
        fileData.size() <
        pixelOffset + pixelCount
    ) {
        throw std::runtime_error(
            "Texture file is too small for decoded pixel data"
        );
    }

    DecodedTexture decoded;

    decoded.width =
        meta.width;

    decoded.height =
        meta.height;

    decoded.pixels.resize(
        pixelCount * 4
    );

    for (
        std::size_t i = 0;
        i < pixelCount;
        i++
    ) {
        const std::uint8_t paletteIndex =
            fileData[pixelOffset + i];

        if (
            paletteIndex >=
            def.palette.size()
        ) {
            throw std::runtime_error(
                "Texture references invalid palette index"
            );
        }

        const RgbColor& color =
            def.palette[paletteIndex];

        const std::size_t out =
            i * 4;

        decoded.pixels[out + 0] =
            color.r;

        decoded.pixels[out + 1] =
            color.g;

        decoded.pixels[out + 2] =
            color.b;

        decoded.pixels[out + 3] =
            paletteIndex == 0
                ? 0
                : 255;
    }

    return decoded;
}

}
