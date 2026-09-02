#include "DefinitionArchive.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

#include "archive/ArchiveParser.h"

namespace eld::definition {

eld::archive::Archive
DefinitionArchive::loadArchive(
    const eld::cache::Store& store,
    std::uint16_t archiveId
) {
    const eld::cache::File cacheFile =
        store.get(
            archiveId
        );

    eld::archive::ArchiveParser parser;

    std::optional<eld::archive::Archive> archive =
        parser.parse(
            cacheFile.getBytes()
        );

    if (!archive.has_value()) {
        throw std::runtime_error(
            "Failed to parse definition archive"
        );
    }

    return std::move(*archive);
}

DefinitionArchive::DefinitionArchive(
    eld::cache::Store store,
    std::uint16_t archiveId
)
    : archive_(
          loadArchive(
              store,
              archiveId
          )
      ) {
}

DefinitionTable DefinitionArchive::get(
    std::string_view name
) const {
    const std::string dataName =
        std::string(name) +
        ".dat";

    const std::string indexName =
        std::string(name) +
        ".idx";

    const eld::archive::ArchiveFile& dataFile =
        archive_.get(
            dataName
        );

    const eld::archive::ArchiveFile& indexFile =
        archive_.get(
            indexName
        );

    std::optional<DefinitionTable> table =
        parser_.parse(
            dataFile.payload,
            indexFile.payload
        );

    if (!table.has_value()) {
        throw std::runtime_error(
            "Failed to parse definition table " +
            std::string(name)
        );
    }

    return std::move(*table);
}

std::optional<DefinitionTable>
DefinitionArchive::find(
    std::string_view name
) const {
    if (!contains(name)) {
        return std::nullopt;
    }

    try {
        return get(name);
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

bool DefinitionArchive::contains(
    std::string_view name
) const {
    return
        archive_.contains(
            std::string(name) +
            ".dat"
        ) &&
        archive_.contains(
            std::string(name) +
            ".idx"
        );
}

}
