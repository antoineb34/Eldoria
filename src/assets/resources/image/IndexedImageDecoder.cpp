#include "IndexedImageDecoder.h"

#include "IndexedImageFileParser.h"

#include <cstddef>
#include <optional>
#include <stdexcept>

namespace eld::image {

Image IndexedImageDecoder::decodeImage(
    const IndexedImageFile& file,
    IndexedImageSourceMap* sourceMap
) const {
    const std::size_t canvasWidth =
        file.metadata.canvasWidth;

    const std::size_t canvasHeight =
        file.metadata.canvasHeight;

    const std::size_t imageWidth =
        file.metadata.width;

    const std::size_t imageHeight =
        file.metadata.height;

    if (
        canvasWidth == 0 ||
        canvasHeight == 0 ||
        imageWidth == 0 ||
        imageHeight == 0
    ) {
        throw std::invalid_argument(
            "Texture dimensions must be positive"
        );
    }

    if (
        static_cast<std::size_t>(
            file.metadata.offsetX
        ) +
        imageWidth >
        canvasWidth ||
        static_cast<std::size_t>(
            file.metadata.offsetY
        ) +
        imageHeight >
        canvasHeight
    ) {
        throw std::invalid_argument(
            "Texture image exceeds its canvas"
        );
    }

    const std::size_t expectedPixelCount =
        imageWidth *
        imageHeight;

    if (
        file.pixels.size() !=
        expectedPixelCount
    ) {
        throw std::invalid_argument(
            "Texture file pixel count is invalid"
        );
    }

    Image image{};

    image.width =
        file.metadata.canvasWidth;

    image.height =
        file.metadata.canvasHeight;

    image.pixels.resize(
        canvasWidth *
        canvasHeight
    );

    if (sourceMap != nullptr) {
        *sourceMap =
            IndexedImageSourceMap{};

        sourceMap->pixels.resize(
            image.pixels.size()
        );
    }

    for (
        std::size_t filePixelIndex = 0;
        filePixelIndex < file.pixels.size();
        filePixelIndex++
    ) {
        std::size_t sourceX = 0;
        std::size_t sourceY = 0;

        switch (file.metadata.pixelOrder) {
            case IndexedImagePixelOrder::RowMajor:
                sourceX =
                    filePixelIndex %
                    imageWidth;

                sourceY =
                    filePixelIndex /
                    imageWidth;

                break;

            case IndexedImagePixelOrder::ColumnMajor:
                sourceX =
                    filePixelIndex /
                    imageHeight;

                sourceY =
                    filePixelIndex %
                    imageHeight;

                break;
        }

        const std::size_t canvasX =
            static_cast<std::size_t>(
                file.metadata.offsetX
            ) +
            sourceX;

        const std::size_t canvasY =
            static_cast<std::size_t>(
                file.metadata.offsetY
            ) +
            sourceY;

        const std::size_t canvasPixelIndex =
            canvasY *
            canvasWidth +
            canvasX;

        const IndexedImagePixel& filePixel =
            file.pixels.at(
                filePixelIndex
            );

        const std::size_t paletteIndex =
            filePixel.paletteIndex;

        if (
            paletteIndex >=
            file.palette.size()
        ) {
            throw std::invalid_argument(
                "Texture palette index is invalid"
            );
        }

        if (paletteIndex != 0) {
            const IndexedImageColor& color =
                file.palette.at(
                    paletteIndex
                );

            image.pixels.at(
                canvasPixelIndex
            ) =
                RgbaPixel{
                    color.red,
                    color.green,
                    color.blue,
                    255
                };
        }

        if (sourceMap != nullptr) {
            sourceMap->pixels.at(
                canvasPixelIndex
            ) =
                IndexedImagePixelSource{
                    filePixelIndex,
                    paletteIndex
                };
        }
    }

    return image;
}

Image IndexedImageDecoder::decode(
    std::span<const std::uint8_t> dataPayload,
    std::span<const std::uint8_t> indexPayload,
    std::uint16_t frameId
) const {
    IndexedImageFileParser parser;

    std::optional<IndexedImageFile> file =
        parser.parse(
            dataPayload,
            indexPayload,
            frameId
        );

    if (!file.has_value()) {
        throw std::runtime_error(
            "Invalid indexed-image payload"
        );
    }

    return decode(*file);
}

Image IndexedImageDecoder::decode(
    const IndexedImageFile& file
) const {
    return decodeImage(
        file,
        nullptr
    );
}

Image IndexedImageDecoder::decode(
    const IndexedImageFile& file,
    IndexedImageSourceMap& sourceMap
) const {
    return decodeImage(
        file,
        &sourceMap
    );
}

}
