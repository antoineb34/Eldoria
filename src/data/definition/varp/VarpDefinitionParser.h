#pragma once

#include <optional>

#include "../DefinitionTable.h"
#include "VarpDefinition.h"

namespace eld::definition {

class VarpDefinitionParser {
public:
    std::optional<VarpDefinition> parse(
        const DefinitionRecord& record
    ) const;
};

}
