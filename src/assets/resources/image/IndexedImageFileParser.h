#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "IndexedImageFile.h"

namespace eld::image {

class IndexedImageFileParser {
public:
    std::optional<IndexedImageFile> parse(
        const std::vector<std::uint8_t>& dataPayload,
        const std::vector<std::uint8_t>& indexPayload,
        std::uint16_t frameId = 0
    ) const;
};

}
