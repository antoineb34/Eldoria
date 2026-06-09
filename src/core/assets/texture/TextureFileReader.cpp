#include "TextureFileReader.h"

#include "binary/ByteBuffer.h"

namespace rf::texture {

std::optional<TextureFile> TextureFileReader::read(
    int id,
    const std::vector<uint8_t>& indexData,
    const std::vector<uint8_t>& textureData
) const {
    if (textureData.size() < 2) {
        return std::nullopt;
    }

    rf::io::ByteBuffer textureBuffer(textureData);

    int indexOffset =
        textureBuffer.readU16();

    if (
        indexOffset < 0 ||
        indexOffset + 5 >= static_cast<int>(indexData.size())
    ) {
        return std::nullopt;
    }

    int paletteCount =
        indexData[indexOffset + 4];

    TextureFile file {};

    file.id = id;

    file.palette =
        readPalette(
            indexData,
            indexOffset,
            paletteCount
        );

    file.metadata =
        readMetadata(
            indexData,
            indexOffset,
            paletteCount
        );

    file.indexedPixels =
        readIndexedPixels(
            textureData,
            file.metadata
        );

    return file;
}

TexturePalette TextureFileReader::readPalette(
    const std::vector<uint8_t>& indexData,
    int indexOffset,
    int paletteCount
) const {
    TexturePalette palette {};

    palette.colors.resize(
        paletteCount
    );

    palette.colors[0] = {
        0,
        0,
        0
    };

    int paletteOffset =
        indexOffset + 5;

    for (int i = 1; i < paletteCount; i++) {
        int offset =
            paletteOffset +
            (i - 1) * 3;

        palette.colors[i] = {
            indexData[offset],
            indexData[offset + 1],
            indexData[offset + 2]
        };
    }

    return palette;
}

TextureMetadata TextureFileReader::readMetadata(
    const std::vector<uint8_t>& indexData,
    int indexOffset,
    int paletteCount
) const {
    TextureMetadata metadata {};

    metadata.canvasWidth =
        static_cast<int>(
            (
                static_cast<uint16_t>(
                    indexData[indexOffset]
                ) << 8
            ) |
            indexData[indexOffset + 1]
        );

    metadata.canvasHeight =
        static_cast<int>(
            (
                static_cast<uint16_t>(
                    indexData[indexOffset + 2]
                ) << 8
            ) |
            indexData[indexOffset + 3]
        );

    int metadataOffset =
        indexOffset +
        5 +
        (paletteCount - 1) * 3;

    metadata.xOffset =
        indexData[metadataOffset];

    metadata.yOffset =
        indexData[metadataOffset + 1];

    metadata.width =
        static_cast<int>(
            (
                static_cast<uint16_t>(
                    indexData[metadataOffset + 2]
                ) << 8
            ) |
            indexData[metadataOffset + 3]
        );

    metadata.height =
        static_cast<int>(
            (
                static_cast<uint16_t>(
                    indexData[metadataOffset + 4]
                ) << 8
            ) |
            indexData[metadataOffset + 5]
        );

    metadata.type =
        indexData[metadataOffset + 6];

    return metadata;
}

std::vector<uint8_t> TextureFileReader::readIndexedPixels(
    const std::vector<uint8_t>& textureData,
    const TextureMetadata& metadata
) const {
    int pixelCount =
        metadata.width *
        metadata.height;

    std::vector<uint8_t> pixels;

    pixels.reserve(
        pixelCount
    );

    for (int i = 0; i < pixelCount; i++) {
        pixels.push_back(
            textureData[2 + i]
        );
    }

    return pixels;
}

}