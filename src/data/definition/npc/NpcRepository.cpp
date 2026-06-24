#include "NpcRepository.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "NpcDefinitionParser.h"

namespace eld::definition {

NpcRepository::NpcRepository(
    DefinitionTable table
)
    : table_(std::move(table)) {
    definitions_.reserve(
        table_.count()
    );

    NpcDefinitionParser parser;

    for (
        const DefinitionRecord& record :
        table_.list()
    ) {
        std::optional<NpcDefinition> definition =
            parser.parse(record);

        if (!definition.has_value()) {
            throw std::runtime_error(
                "Failed to parse NPC definition " +
                std::to_string(record.id)
            );
        }

        definitions_.push_back(
            std::move(*definition)
        );
    }
}

const NpcDefinition& NpcRepository::get(
    std::uint16_t id
) const {
    const NpcDefinition* definition =
        find(id);

    if (definition == nullptr) {
        throw std::out_of_range(
            "NPC definition does not exist"
        );
    }

    return *definition;
}

const NpcDefinition* NpcRepository::find(
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

const std::vector<NpcDefinition>&
NpcRepository::list() const {
    return definitions_;
}

bool NpcRepository::contains(
    std::uint16_t id
) const {
    return find(id) != nullptr;
}

std::size_t NpcRepository::count() const {
    return definitions_.size();
}

}
