#include "IndexedImageFileParser.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <utility>

#include "binary/ByteReader.h"

namespace eld::image {

namespace {

std::vector<IndexedImageColor> readPalette(
    eld::binary::ByteReader& reader
) {
    const std::uint8_t colorCount =
        reader.readU8();

    if (colorCount == 0) {
        return {};
    }

    std::vector<IndexedImageColor> palette;

    palette.reserve(
        colorCount
    );

    palette.push_back(
        IndexedImageColor{}
    );

    for (
        std::size_t i = 1;
        i < colorCount;
        i++
    ) {
        const std::uint32_t color =
            reader.readU24();

        palette.push_back(
            IndexedImageColor{
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

std::optional<IndexedImagePixelOrder> readPixelOrder(
    eld::binary::ByteReader& reader
) {
    const std::uint8_t value =
        reader.readU8();

    switch (value) {
        case 0:
            return IndexedImagePixelOrder::RowMajor;

        case 1:
            return IndexedImagePixelOrder::ColumnMajor;

        default:
            return std::nullopt;
    }
}

std::vector<IndexedImagePixel> readPixels(
    eld::binary::ByteReader& reader,
    std::size_t pixelCount
) {
    std::vector<IndexedImagePixel> pixels;

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
            IndexedImagePixel{
                sourceOffset,
                reader.readU8()
            }
        );
    }

    return pixels;
}

}

std::optional<IndexedImageFile> IndexedImageFileParser::parse(
    std::span<const std::uint8_t> dataPayload,
    std::span<const std::uint8_t> indexPayload,
    std::uint16_t frameId
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

        IndexedImageMetadata metadata{};

        metadata.indexOffset =
            indexOffset;

        metadata.canvasWidth =
            indexReader.readU16();

        metadata.canvasHeight =
            indexReader.readU16();

        std::vector<IndexedImageColor> palette =
            readPalette(
                indexReader
            );

        if (palette.empty()) {
            return std::nullopt;
        }

        for (
            std::uint16_t currentFrame = 0;
            currentFrame < frameId;
            currentFrame++
        ) {
            indexReader.skip(2);

            const std::size_t previousWidth =
                indexReader.readU16();

            const std::size_t previousHeight =
                indexReader.readU16();

            indexReader.skip(1);

            const std::size_t previousPixelCount =
                previousWidth *
                previousHeight;

            if (
                !dataReader.canRead(
                    previousPixelCount
                )
            ) {
                return std::nullopt;
            }

            dataReader.skip(
                previousPixelCount
            );
        }

        metadata.frameId =
            frameId;

        metadata.offsetX =
            indexReader.readU8();

        metadata.offsetY =
            indexReader.readU8();

        metadata.width =
            indexReader.readU16();

        metadata.height =
            indexReader.readU16();

        const std::optional<IndexedImagePixelOrder> pixelOrder =
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

        std::vector<IndexedImagePixel> pixels =
            readPixels(
                dataReader,
                pixelCount
            );

        for (const IndexedImagePixel& pixel : pixels) {
            if (
                pixel.paletteIndex >=
                palette.size()
            ) {
                return std::nullopt;
            }
        }

        return IndexedImageFile{
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
