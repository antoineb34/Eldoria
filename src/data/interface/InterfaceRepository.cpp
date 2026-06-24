#include "InterfaceRepository.h"

#include <optional>
#include <stdexcept>
#include <utility>

#include "archive/Archive.h"
#include "archive/ArchiveParser.h"
#include "cache/File.h"

namespace eld::interface {

InterfaceRepository::InterfaceRepository(
    eld::cache::Store store,
    std::uint16_t archiveId
) {
    const eld::cache::File cacheFile =
        store.get(archiveId);

    eld::archive::ArchiveParser archiveParser;

    std::optional<eld::archive::Archive> archive =
        archiveParser.parse(
            cacheFile.getBytes()
        );

    if (!archive.has_value()) {
        throw std::runtime_error(
            "Failed to parse interface archive"
        );
    }

    const eld::archive::ArchiveFile& dataFile =
        archive->get("data");

    InterfaceParser parser;

    std::optional<std::vector<InterfaceDefinition>>
        definitions =
            parser.parse(dataFile.payload);

    if (!definitions.has_value()) {
        throw std::runtime_error(
            "Failed to parse interface definitions"
        );
    }

    definitions_ =
        std::move(*definitions);
}

const InterfaceDefinition& InterfaceRepository::get(
    std::uint16_t id
) const {
    const InterfaceDefinition* definition =
        find(id);

    if (definition == nullptr) {
        throw std::out_of_range(
            "Interface definition does not exist"
        );
    }

    return *definition;
}

const InterfaceDefinition* InterfaceRepository::find(
    std::uint16_t id
) const {
    for (
        const InterfaceDefinition& definition :
        definitions_
    ) {
        if (definition.id == id) {
            return &definition;
        }
    }

    return nullptr;
}

const std::vector<InterfaceDefinition>&
InterfaceRepository::list() const {
    return definitions_;
}

bool InterfaceRepository::contains(
    std::uint16_t id
) const {
    return find(id) != nullptr;
}

std::size_t InterfaceRepository::count() const {
    return definitions_.size();
}

}
