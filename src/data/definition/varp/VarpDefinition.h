#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace eld::definition {

struct VarpDefinition {
    std::uint16_t id = 0;

    std::optional<std::uint8_t> opcode1Value;
    std::optional<std::uint8_t> opcode2Value;

    bool tracked = false;
    bool persistent = true;

    std::optional<std::uint16_t> clientCode;

    bool opcode6Flag = false;
    std::optional<std::uint32_t> opcode7Value;

    bool active = false;
    std::int8_t mode = -1;

    std::string name;
    std::optional<std::uint32_t> opcode12Value;
};

}
