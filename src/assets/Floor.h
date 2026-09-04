#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace eld::floor {

struct Floor {
    std::uint16_t id = 0;

    std::optional<std::uint32_t> rgb;
    std::optional<std::uint8_t> textureId;
    std::optional<std::uint32_t> secondaryRgb;

    bool occlude = true;
    std::string name;
};

}
