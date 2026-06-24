#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "FontFile.h"

namespace eld::font {

class FontFileParser {
public:
    std::optional<FontFile> parse(
        const std::vector<std::uint8_t>& dataPayload,
        const std::vector<std::uint8_t>& indexPayload
    ) const;
};

}
