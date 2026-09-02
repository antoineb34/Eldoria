#pragma once

#include <optional>

#include "definition/DefinitionTable.h"
#include "ParameterDefinition.h"

namespace eld::definition {

class ParameterDefinitionParser {
public:
    std::optional<ParameterDefinition> parse(
        const DefinitionRecord& record
    ) const;
};

}
