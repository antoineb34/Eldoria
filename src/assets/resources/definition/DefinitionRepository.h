#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "DefinitionTable.h"
#include "DefinitionTableParser.h"

#include "archive/Archive.h"
#include "cache/Store.h"

namespace eld::definition {

class DefinitionRepository {
public:
    DefinitionRepository(
        eld::cache::Store store,
        std::uint16_t archiveId
    );

    DefinitionTable get(
        std::string_view name
    ) const;

    std::optional<DefinitionTable> find(
        std::string_view name
    ) const;

    bool contains(
        std::string_view name
    ) const;

private:
    static eld::archive::Archive loadArchive(
        const eld::cache::Store& store,
        std::uint16_t archiveId
    );

private:
    eld::archive::Archive archive_;
    DefinitionTableParser parser_;
};

}
