#include "FontFileParser.h"

#include <cstddef>
#include <exception>
#include <utility>

#include "binary/ByteReader.h"

namespace eld::font {

std::optional<FontFile> FontFileParser::parse(
    std::span<const std::uint8_t> dataPayload,
    std::span<const std::uint8_t> indexPayload
) const {
    try {
        eld::binary::ByteReader dataReader(
            dataPayload
        );

        eld::binary::ByteReader indexReader(
            indexPayload
        );

        const std::size_t indexOffset =
            static_cast<std::size_t>(
                dataReader.readU16()
            ) +
            4;

        indexReader.setPosition(
            indexOffset
        );

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

        std::vector<FontGlyphFile> glyphs;

        glyphs.reserve(
            256
        );

        for (
            std::uint16_t character = 0;
            character < 256;
            character++
        ) {
            FontGlyphFile glyph;

            glyph.character =
                character;

            glyph.offsetX =
                indexReader.readU8();

            glyph.offsetY =
                indexReader.readU8();

            glyph.width =
                indexReader.readU16();

            glyph.height =
                indexReader.readU16();

            const std::uint8_t pixelOrder =
                indexReader.readU8();

            if (pixelOrder > 1) {
                return std::nullopt;
            }

            glyph.pixelOrder =
                static_cast<FontPixelOrder>(
                    pixelOrder
                );

            const std::size_t pixelCount =
                static_cast<std::size_t>(
                    glyph.width
                ) *
                static_cast<std::size_t>(
                    glyph.height
                );

            if (!dataReader.canRead(pixelCount)) {
                return std::nullopt;
            }

            glyph.pixels =
                dataReader.readBytes(
                    pixelCount
                );

            glyphs.push_back(
                std::move(glyph)
            );
        }

        return FontFile{
            .glyphs = std::move(glyphs)
        };
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

}
