#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "../DefinitionTable.h"
#include "ParameterDefinition.h"

namespace eld::definition {

class ParameterRepository {
public:
    explicit ParameterRepository(
        DefinitionTable table
    );

    const ParameterDefinition& get(
        std::uint16_t id
    ) const;

    const ParameterDefinition* find(
        std::uint16_t id
    ) const;

    const std::vector<ParameterDefinition>&
    list() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    DefinitionTable table_;
    std::vector<ParameterDefinition> definitions_;
};

}
