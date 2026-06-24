#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "DefinitionTable.h"

namespace eld::definition {

class DefinitionTableParser {
public:
    std::optional<DefinitionTable> parse(
        const std::vector<std::uint8_t>& dataPayload,
        const std::vector<std::uint8_t>& indexPayload
    ) const;
};

}
