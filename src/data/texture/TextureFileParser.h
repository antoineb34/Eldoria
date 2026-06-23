#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "TextureFile.h"

namespace eld::texture {

class TextureFileParser {
public:
    std::optional<TextureFile> parse(
        const std::vector<std::uint8_t>& dataPayload,
        const std::vector<std::uint8_t>& indexPayload
    ) const;
};

}
