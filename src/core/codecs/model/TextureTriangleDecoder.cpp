#include "TextureTriangleDecoder.h"

#include "../../io/ByteBuffer.h"

namespace rf::model {

std::vector<TextureTriangle> decodeTextureTriangles(
    const std::vector<char>& payload,
    const ModelFooter& footer,
    const ModelLayout& layout
) {
    std::vector<TextureTriangle> textureTriangles;

    textureTriangles.reserve(
        footer.textureTriangleCount
    );

    auto readU16 =
        [&](std::size_t offset) -> uint16_t {
            return
                (
                    static_cast<uint8_t>(
                        payload[offset]
                    ) << 8
                ) |
                static_cast<uint8_t>(
                    payload[offset + 1]
                );
        };

    std::size_t offset =
        layout.textureDataOffset;

    for (
        uint32_t i = 0;
        i < footer.textureTriangleCount;
        i++
    ) {
        TextureTriangle triangle;

        triangle.a =
            readU16(offset);

        triangle.b =
            readU16(offset + 2);

        triangle.c =
            readU16(offset + 4);

        offset += 6;

        textureTriangles.push_back(
            triangle
        );
    }

    return textureTriangles;
}

}
