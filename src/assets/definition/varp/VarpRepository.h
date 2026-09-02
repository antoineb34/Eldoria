#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "../DefinitionTable.h"
#include "VarpDefinition.h"

namespace eld::definition {

class VarpRepository {
public:
    explicit VarpRepository(
        DefinitionTable table
    );

    const VarpDefinition& get(
        std::uint16_t id
    ) const;

    const VarpDefinition* find(
        std::uint16_t id
    ) const;

    const std::vector<VarpDefinition>&
    list() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    DefinitionTable table_;
    std::vector<VarpDefinition> definitions_;
};

}
