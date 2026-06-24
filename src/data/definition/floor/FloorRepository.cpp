#include "FloorRepository.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace eld::definition {

FloorRepository::FloorRepository(
    DefinitionTable table
)
    : table_(std::move(table)) {
    definitions_.reserve(
        table_.count()
    );

    FloorDefinitionParser parser;

    for (
        const DefinitionRecord& record :
        table_.list()
    ) {
        std::optional<FloorDefinition> definition =
            parser.parse(
                record
            );

        if (!definition.has_value()) {
            throw std::runtime_error(
                "Failed to parse floor definition " +
                std::to_string(record.id)
            );
        }

        definitions_.push_back(
            std::move(*definition)
        );
    }
}

const FloorDefinition& FloorRepository::get(
    std::uint16_t id
) const {
    const FloorDefinition* definition =
        find(id);

    if (definition == nullptr) {
        throw std::out_of_range(
            "Floor definition does not exist"
        );
    }

    return *definition;
}

const FloorDefinition* FloorRepository::find(
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

const std::vector<FloorDefinition>&
FloorRepository::list() const {
    return definitions_;
}

bool FloorRepository::contains(
    std::uint16_t id
) const {
    return find(id) != nullptr;
}

std::size_t FloorRepository::count() const {
    return definitions_.size();
}

}
