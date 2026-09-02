#pragma once

#include <optional>

#include "../DefinitionTable.h"
#include "LocationDefinition.h"

namespace eld::definition {

class LocationDefinitionParser {
public:
    std::optional<LocationDefinition> parse(
        const DefinitionRecord& record
    ) const;
};

}
