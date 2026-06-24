#pragma once

#include <optional>

#include "../DefinitionTable.h"
#include "MessageDefinition.h"

namespace eld::definition {

class MessageDefinitionParser {
public:
    std::optional<MessageDefinition> parse(
        const DefinitionRecord& record
    ) const;
};

}
