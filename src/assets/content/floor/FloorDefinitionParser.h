#pragma once

#include <optional>

#include "definition/DefinitionTable.h"
#include "FloorDefinition.h"

namespace eld::definition {

class FloorDefinitionParser {
public:
    std::optional<FloorDefinition> parse(
        const DefinitionRecord& record
    ) const;
};

}
