#pragma once

#include <optional>

#include "definition/DefinitionTable.h"
#include "NpcDefinition.h"

namespace eld::definition {

class NpcDefinitionParser {
public:
    std::optional<NpcDefinition> parse(
        const DefinitionRecord& record
    ) const;
};

}
