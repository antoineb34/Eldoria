#pragma once

#include <optional>

#include "../DefinitionTable.h"
#include "SpotAnimationDefinition.h"

namespace eld::definition {

class SpotAnimationDefinitionParser {
public:
    std::optional<SpotAnimationDefinition> parse(
        const DefinitionRecord& record
    ) const;
};

}
