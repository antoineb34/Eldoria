#include "ParameterRepository.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "ParameterDefinitionParser.h"

namespace eld::definition {

ParameterRepository::ParameterRepository(
    DefinitionTable table
)
    : table_(std::move(table)) {
    definitions_.reserve(
        table_.count()
    );

    ParameterDefinitionParser parser;

    for (
        const DefinitionRecord& record :
        table_.list()
    ) {
        std::optional<ParameterDefinition> definition =
            parser.parse(record);

        if (!definition.has_value()) {
            throw std::runtime_error(
                "Failed to parse parameter definition " +
                std::to_string(record.id)
            );
        }

        definitions_.push_back(
            std::move(*definition)
        );
    }
}

const ParameterDefinition& ParameterRepository::get(
    std::uint16_t id
) const {
    const ParameterDefinition* definition =
        find(id);

    if (definition == nullptr) {
        throw std::out_of_range(
            "Parameter definition does not exist"
        );
    }

    return *definition;
}

const ParameterDefinition* ParameterRepository::find(
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

const std::vector<ParameterDefinition>&
ParameterRepository::list() const {
    return definitions_;
}

bool ParameterRepository::contains(
    std::uint16_t id
) const {
    return find(id) != nullptr;
}

std::size_t ParameterRepository::count() const {
    return definitions_.size();
}

}
