#pragma once

#include <optional>

#include "../DefinitionTable.h"
#include "ParameterDefinition.h"

namespace eld::definition {

class ParameterDefinitionParser {
public:
    std::optional<ParameterDefinition> parse(
        const DefinitionRecord& record
    ) const;
};

}
