#include "LocationRepository.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "LocationDefinitionParser.h"

namespace eld::definition {

LocationRepository::LocationRepository(
    DefinitionTable table
)
    : table_(std::move(table)) {
    definitions_.reserve(
        table_.count()
    );

    LocationDefinitionParser parser;

    for (
        const DefinitionRecord& record :
        table_.list()
    ) {
        std::optional<LocationDefinition> definition =
            parser.parse(record);

        if (!definition.has_value()) {
            throw std::runtime_error(
                "Failed to parse location definition " +
                std::to_string(record.id)
            );
        }

        definitions_.push_back(
            std::move(*definition)
        );
    }
}

const LocationDefinition& LocationRepository::get(
    std::uint16_t id
) const {
    const LocationDefinition* definition =
        find(id);

    if (definition == nullptr) {
        throw std::out_of_range(
            "Location definition does not exist"
        );
    }

    return *definition;
}

const LocationDefinition* LocationRepository::find(
    std::uint16_t id
) const {
    if (
        static_cast<std::size_t>(id) >=
        definitions_.size()
    ) {
        return nullptr;
    }

    return &definitions_[id];
}

const std::vector<LocationDefinition>&
LocationRepository::list() const {
    return definitions_;
}

bool LocationRepository::contains(
    std::uint16_t id
) const {
    return find(id) != nullptr;
}

std::size_t LocationRepository::count() const {
    return definitions_.size();
}

}
