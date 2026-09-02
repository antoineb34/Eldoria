#pragma once

#include <cstdint>
#include <span>

#include "IndexedImageFile.h"
#include "Image.h"
#include "IndexedImageSourceMap.h"

namespace eld::image {

class IndexedImageDecoder {
public:
    Image decode(
        std::span<const std::uint8_t> dataPayload,
        std::span<const std::uint8_t> indexPayload,
        std::uint16_t frameId = 0
    ) const;

    Image decode(
        const IndexedImageFile& file
    ) const;

    Image decode(
        const IndexedImageFile& file,
        IndexedImageSourceMap& sourceMap
    ) const;

private:
    Image decodeImage(
        const IndexedImageFile& file,
        IndexedImageSourceMap* sourceMap
    ) const;
};

}
