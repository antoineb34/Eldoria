#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "InterfaceDefinition.h"

namespace eld::interface {

class InterfaceParser {
public:
    std::optional<std::vector<InterfaceDefinition>> parse(
        const std::vector<std::uint8_t>& payload
    ) const;
};

}
