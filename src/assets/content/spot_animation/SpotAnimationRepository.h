#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "definition/DefinitionTable.h"
#include "SpotAnimationDefinition.h"

namespace eld::definition {

class SpotAnimationRepository {
public:
    explicit SpotAnimationRepository(
        DefinitionTable table
    );

    const SpotAnimationDefinition& get(
        std::uint16_t id
    ) const;

    const SpotAnimationDefinition* find(
        std::uint16_t id
    ) const;

    const std::vector<SpotAnimationDefinition>&
    list() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    DefinitionTable table_;
    std::vector<SpotAnimationDefinition> definitions_;
};

}
