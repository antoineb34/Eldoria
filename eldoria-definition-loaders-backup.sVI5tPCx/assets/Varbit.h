#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace eld::varbit {

struct Varbit {
    std::uint16_t id = 0;

    std::optional<std::uint16_t> varpId;
    std::uint8_t leastSignificantBit = 0;
    std::uint8_t mostSignificantBit = 0;

    bool tracked = false;

    std::optional<std::uint32_t> opcode3Value;
    std::optional<std::uint32_t> opcode4Value;

    std::string name;
};

}
