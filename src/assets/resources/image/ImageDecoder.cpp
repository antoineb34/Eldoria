#include "ImageDecoder.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "binary/ByteReader.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#include <stb/stb_image.h>

namespace eld::image {

namespace {

enum class PixelOrder : std::uint8_t {
    RowMajor,
    ColumnMajor
};

struct Color {
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;
};


std::vector<Color> readPalette(
    eld::binary::ByteReader& reader
) {
    const auto colorCount = reader.readU8();

    if (colorCount == 0) {
        throw std::runtime_error(
            "Image palette is empty"
        );
    }

    std::vector<Color> palette(colorCount);

    for (std::size_t i = 1; i < colorCount; ++i) {
        const auto color = reader.readU24();

        palette[i] = {
            static_cast<std::uint8_t>((color >> 16) & 0xFF),
            static_cast<std::uint8_t>((color >> 8) & 0xFF),
            static_cast<std::uint8_t>(color & 0xFF)
        };
    }

    return palette;
}


PixelOrder readPixelOrder(
    eld::binary::ByteReader& reader
) {
    switch (reader.readU8()) {
        case 0:
            return PixelOrder::RowMajor;

        case 1:
            return PixelOrder::ColumnMajor;

        default:
            throw std::runtime_error(
                "Invalid image pixel order"
            );
    }
}


void skipFrame(
    eld::binary::ByteReader& data,
    eld::binary::ByteReader& index
) {
    index.skip(2);

    const auto width = index.readU16();
    const auto height = index.readU16();

    index.skip(1);

    const std::size_t pixelCount =
        static_cast<std::size_t>(width) * height;

    if (!data.canRead(pixelCount)) {
        throw std::runtime_error(
            "Image frame exceeds data payload"
        );
    }

    data.skip(pixelCount);
}

}


Image ImageDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    if (payload.empty()) {
        throw std::runtime_error(
            "Image payload is empty"
        );
    }

    if (payload.size() > std::numeric_limits<int>::max()) {
        throw std::runtime_error(
            "Image payload is too large"
        );
    }


    // Decode

    int width = 0;
    int height = 0;
    int channels = 0;

    using Pixels =
        std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;

    Pixels pixels(
        stbi_load_from_memory(
            reinterpret_cast<const stbi_uc*>(payload.data()),
            static_cast<int>(payload.size()),
            &width,
            &height,
            &channels,
            STBI_rgb_alpha
        ),
        stbi_image_free
    );

    if (!pixels) {
        const char* reason = stbi_failure_reason();

        throw std::runtime_error(
            "Failed to decode image: " +
            std::string(reason ? reason : "unknown error")
        );
    }

    if (
        width <= 0 ||
        height <= 0 ||
        width > std::numeric_limits<std::uint16_t>::max() ||
        height > std::numeric_limits<std::uint16_t>::max()
    ) {
        throw std::runtime_error(
            "Image dimensions are unsupported"
        );
    }


    // Image

    Image image;

    image.width = static_cast<std::uint16_t>(width);
    image.height = static_cast<std::uint16_t>(height);

    const std::size_t pixelCount =
        static_cast<std::size_t>(width) * height;

    image.pixels.resize(pixelCount);

    for (std::size_t i = 0; i < pixelCount; ++i) {
        const std::size_t source = i * 4;

        image.pixels[i] = {
            pixels.get()[source],
            pixels.get()[source + 1],
            pixels.get()[source + 2],
            pixels.get()[source + 3]
        };
    }

    return image;
}


Image ImageDecoder::decode(
    std::span<const std::uint8_t> dataPayload,
    std::span<const std::uint8_t> indexPayload,
    std::uint16_t frameId
) const {
    eld::binary::ByteReader data(dataPayload);

    const auto indexOffset = data.readU16();

    eld::binary::ByteReader index(indexPayload);
    index.setPosition(indexOffset);


    // Header

    const auto canvasWidth = index.readU16();
    const auto canvasHeight = index.readU16();

    const std::vector<Color> palette =
        readPalette(index);


    // Frame

    for (std::uint16_t i = 0; i < frameId; ++i) {
        skipFrame(data, index);
    }

    const auto offsetX = index.readU8();
    const auto offsetY = index.readU8();

    const auto width = index.readU16();
    const auto height = index.readU16();

    const PixelOrder pixelOrder =
        readPixelOrder(index);

    if (
        canvasWidth == 0 ||
        canvasHeight == 0 ||
        width == 0 ||
        height == 0
    ) {
        throw std::runtime_error(
            "Image dimensions must be positive"
        );
    }

    if (
        static_cast<std::size_t>(offsetX) + width > canvasWidth ||
        static_cast<std::size_t>(offsetY) + height > canvasHeight
    ) {
        throw std::runtime_error(
            "Image exceeds its canvas"
        );
    }

    const std::size_t pixelCount =
        static_cast<std::size_t>(width) * height;

    if (!data.canRead(pixelCount)) {
        throw std::runtime_error(
            "Image pixel data exceeds payload"
        );
    }


    // Image

    Image image;

    image.width = canvasWidth;
    image.height = canvasHeight;

    image.pixels.resize(
        static_cast<std::size_t>(canvasWidth) * canvasHeight
    );

    for (std::size_t i = 0; i < pixelCount; ++i) {
        std::size_t x = 0;
        std::size_t y = 0;

        switch (pixelOrder) {
            case PixelOrder::RowMajor:
                x = i % width;
                y = i / width;
                break;

            case PixelOrder::ColumnMajor:
                x = i / height;
                y = i % height;
                break;
        }

        const auto paletteIndex = data.readU8();

        if (paletteIndex >= palette.size()) {
            throw std::runtime_error(
                "Invalid image palette index"
            );
        }

        if (paletteIndex == 0) {
            continue;
        }

        const std::size_t canvasX = offsetX + x;
        const std::size_t canvasY = offsetY + y;

        const std::size_t destination =
            canvasY * canvasWidth + canvasX;

        const Color& color = palette[paletteIndex];

        image.pixels[destination] = {
            color.red,
            color.green,
            color.blue,
            255
        };
    }

    return image;
}

}
