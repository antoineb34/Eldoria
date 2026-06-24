#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "../DefinitionTable.h"
#include "SequenceDefinition.h"

namespace eld::definition {

class SequenceRepository {
public:
    explicit SequenceRepository(
        DefinitionTable table
    );

    const SequenceDefinition& get(
        std::uint16_t id
    ) const;

    const SequenceDefinition* find(
        std::uint16_t id
    ) const;

    const std::vector<SequenceDefinition>&
    list() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    DefinitionTable table_;
    std::vector<SequenceDefinition> definitions_;
};

}
