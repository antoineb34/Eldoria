#pragma once

#include <optional>

#include "../DefinitionTable.h"
#include "ItemDefinition.h"

namespace eld::definition {

class ItemDefinitionParser {
public:
    std::optional<ItemDefinition> parse(
        const DefinitionRecord& record
    ) const;
};

}
