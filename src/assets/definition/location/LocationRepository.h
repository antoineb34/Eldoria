#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "../DefinitionTable.h"
#include "LocationDefinition.h"

namespace eld::definition {

class LocationRepository {
public:
    explicit LocationRepository(
        DefinitionTable table
    );

    const LocationDefinition& get(
        std::uint16_t id
    ) const;

    const LocationDefinition* find(
        std::uint16_t id
    ) const;

    const std::vector<LocationDefinition>&
    list() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    DefinitionTable table_;
    std::vector<LocationDefinition> definitions_;
};

}
