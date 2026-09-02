#pragma once

#include <optional>

#include "definition/DefinitionTable.h"
#include "LocationDefinition.h"

namespace eld::definition {

class LocationDefinitionParser {
public:
    std::optional<LocationDefinition> parse(
        const DefinitionRecord& record
    ) const;
};

}
