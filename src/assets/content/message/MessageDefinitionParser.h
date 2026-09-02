#pragma once

#include <optional>

#include "definition/DefinitionTable.h"
#include "MessageDefinition.h"

namespace eld::definition {

class MessageDefinitionParser {
public:
    std::optional<MessageDefinition> parse(
        const DefinitionRecord& record
    ) const;
};

}
