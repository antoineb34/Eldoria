#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "definition/DefinitionTable.h"
#include "VarbitDefinition.h"

namespace eld::definition {

class VarbitRepository {
public:
    explicit VarbitRepository(
        DefinitionTable table
    );

    const VarbitDefinition& get(
        std::uint16_t id
    ) const;

    const VarbitDefinition* find(
        std::uint16_t id
    ) const;

    const std::vector<VarbitDefinition>&
    list() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    DefinitionTable table_;
    std::vector<VarbitDefinition> definitions_;
};

}
