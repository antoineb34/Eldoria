#pragma once

#include <optional>

#include "definition/DefinitionTable.h"
#include "MessageAnimationDefinition.h"

namespace eld::definition {

class MessageAnimationDefinitionParser {
public:
    std::optional<MessageAnimationDefinition> parse(
        const DefinitionRecord& record
    ) const;
};

}
