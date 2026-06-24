#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "InterfaceDefinition.h"
#include "InterfaceParser.h"
#include "cache/Store.h"

namespace eld::interface {

class InterfaceRepository {
public:
    InterfaceRepository(
        eld::cache::Store store,
        std::uint16_t archiveId
    );

    const InterfaceDefinition& get(
        std::uint16_t id
    ) const;

    const InterfaceDefinition* find(
        std::uint16_t id
    ) const;

    const std::vector<InterfaceDefinition>&
    list() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    std::vector<InterfaceDefinition> definitions_;
};

}
