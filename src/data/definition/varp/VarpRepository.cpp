#include "VarpRepository.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "VarpDefinitionParser.h"

namespace eld::definition {

VarpRepository::VarpRepository(
    DefinitionTable table
)
    : table_(std::move(table)) {
    definitions_.reserve(
        table_.count()
    );

    VarpDefinitionParser parser;

    for (
        const DefinitionRecord& record :
        table_.list()
    ) {
        std::optional<VarpDefinition> definition =
            parser.parse(record);

        if (!definition.has_value()) {
            throw std::runtime_error(
                "Failed to parse Varp definition " +
                std::to_string(record.id)
            );
        }

        definitions_.push_back(
            std::move(*definition)
        );
    }
}

const VarpDefinition& VarpRepository::get(
    std::uint16_t id
) const {
    const VarpDefinition* definition =
        find(id);

    if (definition == nullptr) {
        throw std::out_of_range(
            "Varp definition does not exist"
        );
    }

    return *definition;
}

const VarpDefinition* VarpRepository::find(
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

const std::vector<VarpDefinition>&
VarpRepository::list() const {
    return definitions_;
}

bool VarpRepository::contains(
    std::uint16_t id
) const {
    return find(id) != nullptr;
}

std::size_t VarpRepository::count() const {
    return definitions_.size();
}

}
