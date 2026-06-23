#include "TextureFileParser.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <utility>

#include "binary/ByteReader.h"

namespace eld::texture {

namespace {

std::vector<TextureColor> readPalette(
    eld::binary::ByteReader& reader
) {
    const std::uint8_t colorCount =
        reader.readU8();

    if (colorCount == 0) {
        return {};
    }

    std::vector<TextureColor> palette;

    palette.reserve(
        colorCount
    );

    palette.push_back(
        TextureColor{}
    );

    for (
        std::size_t i = 1;
        i < colorCount;
        i++
    ) {
        const std::uint32_t color =
            reader.readU24();

        palette.push_back(
            TextureColor{
                static_cast<std::uint8_t>(
                    (color >> 16) & 0xFF
                ),
                static_cast<std::uint8_t>(
                    (color >> 8) & 0xFF
                ),
                static_cast<std::uint8_t>(
                    color & 0xFF
                )
            }
        );
    }

    return palette;
}

std::optional<TexturePixelOrder> readPixelOrder(
    eld::binary::ByteReader& reader
) {
    const std::uint8_t value =
        reader.readU8();

    switch (value) {
        case 0:
            return TexturePixelOrder::RowMajor;

        case 1:
            return TexturePixelOrder::ColumnMajor;

        default:
            return std::nullopt;
    }
}

std::vector<TexturePixel> readPixels(
    eld::binary::ByteReader& reader,
    std::size_t pixelCount
) {
    std::vector<TexturePixel> pixels;

    pixels.reserve(
        pixelCount
    );

    for (
        std::size_t i = 0;
        i < pixelCount;
        i++
    ) {
        const std::size_t sourceOffset =
            reader.position();

        pixels.push_back(
            TexturePixel{
                sourceOffset,
                reader.readU8()
            }
        );
    }

    return pixels;
}

}

std::optional<TextureFile> TextureFileParser::parse(
    const std::vector<std::uint8_t>& dataPayload,
    const std::vector<std::uint8_t>& indexPayload
) const {
    try {
        eld::binary::ByteReader dataReader(
            dataPayload
        );

        const std::uint16_t indexOffset =
            dataReader.readU16();

        eld::binary::ByteReader indexReader(
            indexPayload
        );

        indexReader.setPosition(
            indexOffset
        );

        TextureMetadata metadata{};

        metadata.indexOffset =
            indexOffset;

        metadata.canvasWidth =
            indexReader.readU16();

        metadata.canvasHeight =
            indexReader.readU16();

        std::vector<TextureColor> palette =
            readPalette(
                indexReader
            );

        if (palette.empty()) {
            return std::nullopt;
        }

        metadata.offsetX =
            indexReader.readU8();

        metadata.offsetY =
            indexReader.readU8();

        metadata.width =
            indexReader.readU16();

        metadata.height =
            indexReader.readU16();

        const std::optional<TexturePixelOrder> pixelOrder =
            readPixelOrder(
                indexReader
            );

        if (!pixelOrder.has_value()) {
            return std::nullopt;
        }

        metadata.pixelOrder =
            *pixelOrder;

        const std::size_t pixelCount =
            static_cast<std::size_t>(
                metadata.width
            ) *
            static_cast<std::size_t>(
                metadata.height
            );

        if (!dataReader.canRead(pixelCount)) {
            return std::nullopt;
        }

        std::vector<TexturePixel> pixels =
            readPixels(
                dataReader,
                pixelCount
            );

        for (const TexturePixel& pixel : pixels) {
            if (
                pixel.paletteIndex >=
                palette.size()
            ) {
                return std::nullopt;
            }
        }

        return TextureFile{
            .dataPayload = dataPayload,
            .indexPayload = indexPayload,
            .metadata = metadata,
            .palette = std::move(palette),
            .pixels = std::move(pixels)
        };
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

}
