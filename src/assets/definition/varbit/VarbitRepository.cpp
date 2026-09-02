#include "VarbitRepository.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "VarbitDefinitionParser.h"

namespace eld::definition {

VarbitRepository::VarbitRepository(
    DefinitionTable table
)
    : table_(std::move(table)) {
    definitions_.reserve(
        table_.count()
    );

    VarbitDefinitionParser parser;

    for (
        const DefinitionRecord& record :
        table_.list()
    ) {
        std::optional<VarbitDefinition> definition =
            parser.parse(record);

        if (!definition.has_value()) {
            throw std::runtime_error(
                "Failed to parse Varbit definition " +
                std::to_string(record.id)
            );
        }

        definitions_.push_back(
            std::move(*definition)
        );
    }
}

const VarbitDefinition& VarbitRepository::get(
    std::uint16_t id
) const {
    const VarbitDefinition* definition =
        find(id);

    if (definition == nullptr) {
        throw std::out_of_range(
            "Varbit definition does not exist"
        );
    }

    return *definition;
}

const VarbitDefinition* VarbitRepository::find(
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

const std::vector<VarbitDefinition>&
VarbitRepository::list() const {
    return definitions_;
}

bool VarbitRepository::contains(
    std::uint16_t id
) const {
    return find(id) != nullptr;
}

std::size_t VarbitRepository::count() const {
    return definitions_.size();
}

}
