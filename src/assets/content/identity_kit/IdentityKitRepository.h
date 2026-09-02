#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "definition/DefinitionTable.h"
#include "IdentityKitDefinition.h"

namespace eld::definition {

class IdentityKitRepository {
public:
    explicit IdentityKitRepository(
        DefinitionTable table
    );

    const IdentityKitDefinition& get(
        std::uint16_t id
    ) const;

    const IdentityKitDefinition* find(
        std::uint16_t id
    ) const;

    const std::vector<IdentityKitDefinition>&
    list() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    DefinitionTable table_;
    std::vector<IdentityKitDefinition> definitions_;
};

}
