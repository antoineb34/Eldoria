#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "../DefinitionTable.h"
#include "NpcDefinition.h"

namespace eld::definition {

class NpcRepository {
public:
    explicit NpcRepository(
        DefinitionTable table
    );

    const NpcDefinition& get(
        std::uint16_t id
    ) const;

    const NpcDefinition* find(
        std::uint16_t id
    ) const;

    const std::vector<NpcDefinition>&
    list() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    DefinitionTable table_;
    std::vector<NpcDefinition> definitions_;
};

}
