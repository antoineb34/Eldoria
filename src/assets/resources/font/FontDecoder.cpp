#include "FontDecoder.h"

#include "FontFileParser.h"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <utility>

namespace eld::font {

Glyph FontDecoder::decodeGlyph(
    const FontGlyphFile& file
) const {
    const std::size_t width =
        file.width;

    const std::size_t height =
        file.height;

    const std::size_t pixelCount =
        width *
        height;

    if (file.pixels.size() != pixelCount) {
        throw std::invalid_argument(
            "Font glyph pixel count is invalid"
        );
    }

    Glyph glyph;

    glyph.character =
        file.character;

    glyph.offsetX = 1;
    glyph.offsetY =
        file.offsetY;

    glyph.width =
        file.width;

    glyph.height =
        file.height;

    glyph.advance =
        static_cast<std::uint16_t>(
            file.width + 2
        );

    glyph.alpha.resize(
        pixelCount
    );

    for (
        std::size_t sourceIndex = 0;
        sourceIndex < pixelCount;
        sourceIndex++
    ) {
        std::size_t x = 0;
        std::size_t y = 0;

        switch (file.pixelOrder) {
            case FontPixelOrder::RowMajor:
                x = sourceIndex % width;
                y = sourceIndex / width;
                break;

            case FontPixelOrder::ColumnMajor:
                x = sourceIndex / height;
                y = sourceIndex % height;
                break;
        }

        const std::size_t destination =
            y *
            width +
            x;

        glyph.alpha[destination] =
            file.pixels[sourceIndex] == 0
                ? 0
                : 255;
    }

    if (
        width == 0 ||
        height == 0
    ) {
        return glyph;
    }

    const std::size_t threshold =
        height / 7;

    std::size_t leftPixels = 0;
    std::size_t rightPixels = 0;

    for (
        std::size_t y = threshold;
        y < height;
        y++
    ) {
        if (glyph.alpha[y * width] != 0) {
            leftPixels++;
        }

        if (
            glyph.alpha[
                y * width +
                width - 1
            ] != 0
        ) {
            rightPixels++;
        }
    }

    if (leftPixels <= threshold) {
        glyph.offsetX = 0;

        if (glyph.advance > 0) {
            glyph.advance--;
        }
    }

    if (
        rightPixels <= threshold &&
        glyph.advance > 0
    ) {
        glyph.advance--;
    }

    return glyph;
}

Font FontDecoder::decodeFile(
    const FontFile& file,
    std::string name
) const {
    std::vector<Glyph> glyphs;

    glyphs.reserve(
        file.glyphs.size()
    );

    std::uint16_t lineHeight = 0;

    for (
        const FontGlyphFile& glyphFile :
        file.glyphs
    ) {
        Glyph glyph =
            decodeGlyph(
                glyphFile
            );

        if (glyph.character < 128) {
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
        .name = std::move(name),
        .lineHeight = lineHeight,
        .glyphs = std::move(glyphs)
    };
}

Font FontDecoder::decode(
    std::span<const std::uint8_t> dataPayload,
    std::span<const std::uint8_t> indexPayload,
    std::string name
) const {
    FontFileParser parser;

    std::optional<FontFile> file =
        parser.parse(
            dataPayload,
            indexPayload
        );

    if (!file.has_value()) {
        throw std::runtime_error(
            "Invalid font payload"
        );
    }

    return decodeFile(*file, std::move(name));
}

}
