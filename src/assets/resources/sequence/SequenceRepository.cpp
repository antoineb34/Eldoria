#include "SequenceRepository.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "SequenceDefinitionParser.h"

namespace eld::definition {

SequenceRepository::SequenceRepository(
    DefinitionTable table
)
    : table_(std::move(table)) {
    definitions_.reserve(
        table_.count()
    );

    SequenceDefinitionParser parser;

    for (
        const DefinitionRecord& record :
        table_.list()
    ) {
        std::optional<SequenceDefinition> definition =
            parser.parse(record);

        if (!definition.has_value()) {
            throw std::runtime_error(
                "Failed to parse sequence definition " +
                std::to_string(record.id)
            );
        }

        definitions_.push_back(
            std::move(*definition)
        );
    }
}

const SequenceDefinition& SequenceRepository::get(
    std::uint16_t id
) const {
    const SequenceDefinition* definition =
        find(id);

    if (definition == nullptr) {
        throw std::out_of_range(
            "Sequence definition does not exist"
        );
    }

    return *definition;
}

const SequenceDefinition* SequenceRepository::find(
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

const std::vector<SequenceDefinition>&
SequenceRepository::list() const {
    return definitions_;
}

bool SequenceRepository::contains(
    std::uint16_t id
) const {
    return find(id) != nullptr;
}

std::size_t SequenceRepository::count() const {
    return definitions_.size();
}

}
