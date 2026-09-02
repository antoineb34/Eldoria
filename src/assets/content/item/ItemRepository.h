#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "definition/DefinitionTable.h"
#include "ItemDefinition.h"

namespace eld::definition {

class ItemRepository {
public:
    explicit ItemRepository(
        DefinitionTable table
    );

    const ItemDefinition& get(
        std::uint16_t id
    ) const;

    const ItemDefinition* find(
        std::uint16_t id
    ) const;

    const std::vector<ItemDefinition>&
    list() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    DefinitionTable table_;
    std::vector<ItemDefinition> definitions_;
};

}
