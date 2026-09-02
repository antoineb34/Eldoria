#pragma once

#include <cstdint>
#include <optional>
#include <span>

#include "FontFile.h"

namespace eld::font {

class FontFileParser {
public:
    std::optional<FontFile> parse(
        std::span<const std::uint8_t> dataPayload,
        std::span<const std::uint8_t> indexPayload
    ) const;
};

}
