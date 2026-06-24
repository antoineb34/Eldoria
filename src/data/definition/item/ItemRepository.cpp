#include "ItemRepository.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "ItemDefinitionParser.h"

namespace eld::definition {

ItemRepository::ItemRepository(
    DefinitionTable table
)
    : table_(std::move(table)) {
    definitions_.reserve(
        table_.count()
    );

    ItemDefinitionParser parser;

    for (
        const DefinitionRecord& record :
        table_.list()
    ) {
        std::optional<ItemDefinition> definition =
            parser.parse(record);

        if (!definition.has_value()) {
            throw std::runtime_error(
                "Failed to parse item definition " +
                std::to_string(record.id)
            );
        }

        definitions_.push_back(
            std::move(*definition)
        );
    }
}

const ItemDefinition& ItemRepository::get(
    std::uint16_t id
) const {
    const ItemDefinition* definition =
        find(id);

    if (definition == nullptr) {
        throw std::out_of_range(
            "Item definition does not exist"
        );
    }

    return *definition;
}

const ItemDefinition* ItemRepository::find(
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

const std::vector<ItemDefinition>&
ItemRepository::list() const {
    return definitions_;
}

bool ItemRepository::contains(
    std::uint16_t id
) const {
    return find(id) != nullptr;
}

std::size_t ItemRepository::count() const {
    return definitions_.size();
}

}
