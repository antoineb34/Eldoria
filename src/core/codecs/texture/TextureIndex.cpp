#include "TextureIndex.h"

#include <stdexcept>

namespace rf::texture {

static std::uint16_t readU16(
    const std::vector<std::uint8_t>& data,
    std::size_t offset
) {
    if (offset + 1 >= data.size()) {
        throw std::runtime_error(
            "Texture index read out of bounds"
        );
    }

    return static_cast<std::uint16_t>(
        (data[offset] << 8) |
        data[offset + 1]
    );
}

TextureIndex TextureIndexParser::parse(
    const std::vector<std::uint8_t>& data
) {
    TextureIndex index;

    index.rawData = data;

    index.canvasWidth =
        readU16(data, 0);

    index.canvasHeight =
        readU16(data, 2);

    const std::uint8_t paletteCount =
        data[4];

    index.palette.resize(
        paletteCount
    );

    index.palette[0] = {
        0,
        0,
        0
    };

    std::size_t offset = 5;

    for (
        std::size_t i = 1;
        i < paletteCount;
        i++
    ) {
        index.palette[i] = {
            data[offset],
            data[offset + 1],
            data[offset + 2]
        };

        offset += 3;
    }

    return index;
}

}
