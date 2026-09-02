#pragma once

#include <optional>

#include "definition/DefinitionTable.h"
#include "SequenceDefinition.h"

namespace eld::definition {

class SequenceDefinitionParser {
public:
    std::optional<SequenceDefinition> parse(
        const DefinitionRecord& record
    ) const;
};

}
