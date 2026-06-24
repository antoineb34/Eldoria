#pragma once

#include <optional>

#include "../DefinitionTable.h"
#include "SequenceDefinition.h"

namespace eld::definition {

class SequenceDefinitionParser {
public:
    std::optional<SequenceDefinition> parse(
        const DefinitionRecord& record
    ) const;
};

}
