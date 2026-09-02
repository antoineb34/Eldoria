#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "definition/DefinitionTable.h"
#include "FloorDefinition.h"
#include "FloorDefinitionParser.h"

namespace eld::definition {

class FloorRepository {
public:
    explicit FloorRepository(
        DefinitionTable table
    );

    const FloorDefinition& get(
        std::uint16_t id
    ) const;

    const FloorDefinition* find(
        std::uint16_t id
    ) const;

    const std::vector<FloorDefinition>&
    list() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    DefinitionTable table_;
    std::vector<FloorDefinition> definitions_;
};

}
