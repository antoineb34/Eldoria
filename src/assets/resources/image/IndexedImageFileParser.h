#pragma once

#include <cstdint>
#include <optional>
#include <span>

#include "IndexedImageFile.h"

namespace eld::image {

class IndexedImageFileParser {
public:
    std::optional<IndexedImageFile> parse(
        std::span<const std::uint8_t> dataPayload,
        std::span<const std::uint8_t> indexPayload,
        std::uint16_t frameId = 0
    ) const;
};

}
