#include "decoders/FontDecoder.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#include "binary/ByteReader.h"

namespace eld::font {

Font FontDecoder::decode(
    std::span<const std::uint8_t> data,
    std::span<const std::uint8_t> index
) const {
    eld::binary::ByteReader dataReader(data);
    eld::binary::ByteReader indexReader(index);

    const std::size_t indexOffset =
        static_cast<std::size_t>(
            dataReader.readU16()
        ) +
        4;

    indexReader.setPosition(indexOffset);

    const std::uint8_t colorCount =
        indexReader.readU8();

    if (colorCount > 0) {
        indexReader.skip(
            static_cast<std::size_t>(
                colorCount - 1
            ) *
            3
        );
    }


    // Font

    std::vector<Glyph> glyphs;
    glyphs.reserve(256);

    std::uint16_t lineHeight = 0;


    // Glyphs

    for (
        std::uint16_t character = 0;
        character < 256;
        ++character
    ) {

        indexReader.readU8();

        const auto offsetY = indexReader.readU8();
        const auto width = indexReader.readU16();
        const auto height = indexReader.readU16();
        const auto pixelOrder = indexReader.readU8();

        if (pixelOrder > 1) {
            throw std::runtime_error(
                "Invalid font pixel order"
            );
        }

        const std::size_t pixelCount =
            static_cast<std::size_t>(width) *
            static_cast<std::size_t>(height);

        if (!dataReader.canRead(pixelCount)) {
            throw std::runtime_error(
                "Invalid font pixel data"
            );
        }

        const std::vector<std::uint8_t> pixels =
            dataReader.readBytes(pixelCount);

        Glyph glyph;

        glyph.character = character;
        glyph.offsetX = 1;
        glyph.offsetY = offsetY;
        glyph.width = width;
        glyph.height = height;

        glyph.advance =
            static_cast<std::uint16_t>(
                width + 2
            );

        glyph.alpha.resize(pixelCount);


        // Pixels

        for (
            std::size_t sourceIndex = 0;
            sourceIndex < pixelCount;
            ++sourceIndex
        ) {
            std::size_t x = 0;
            std::size_t y = 0;

            if (pixelOrder == 0) {
                x = sourceIndex % width;
                y = sourceIndex / width;
            }
            else {
                x = sourceIndex / height;
                y = sourceIndex % height;
            }

            const std::size_t destination =
                y * width + x;

            glyph.alpha[destination] =
                pixels[sourceIndex] == 0
                    ? 0
                    : 255;
        }


        // Spacing

        if (
            width > 0 &&
            height > 0
        ) {
            const std::size_t threshold =
                height / 7;

            std::size_t leftPixels = 0;
            std::size_t rightPixels = 0;

            for (
                std::size_t y = threshold;
                y < height;
                ++y
            ) {
                if (glyph.alpha[y * width] != 0) {
                    ++leftPixels;
                }

                if (
                    glyph.alpha[
                        y * width +
                        width - 1
                    ] != 0
                ) {
                    ++rightPixels;
                }
            }

            if (leftPixels <= threshold) {
                glyph.offsetX = 0;

                if (glyph.advance > 0) {
                    --glyph.advance;
                }
            }

            if (
                rightPixels <= threshold &&
                glyph.advance > 0
            ) {
                --glyph.advance;
            }
        }

        if (character < 128) {
            lineHeight =
                std::max(
                    lineHeight,
                    glyph.height
                );
        }

        glyphs.push_back(
            std::move(glyph)
        );
    }

    return Font{
        .lineHeight = lineHeight,
        .glyphs = std::move(glyphs)
    };
}

}
