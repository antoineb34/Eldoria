#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "IdentityKitDefinition.h"

namespace eld::definition {

class IdentityKitDefinitionParser {
public:
    std::optional<IdentityKitDefinition> parse(
        std::uint16_t id,
        const std::vector<std::uint8_t>& data
    ) const;
};

}
