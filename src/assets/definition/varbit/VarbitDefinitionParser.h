#pragma once

#include <optional>

#include "../DefinitionTable.h"
#include "VarbitDefinition.h"

namespace eld::definition {

class VarbitDefinitionParser {
public:
    std::optional<VarbitDefinition> parse(
        const DefinitionRecord& record
    ) const;
};

}
