#include "SpotAnimationRepository.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "SpotAnimationDefinitionParser.h"

namespace eld::definition {

SpotAnimationRepository::SpotAnimationRepository(
    DefinitionTable table
)
    : table_(std::move(table)) {
    definitions_.reserve(
        table_.count()
    );

    SpotAnimationDefinitionParser parser;

    for (
        const DefinitionRecord& record :
        table_.list()
    ) {
        std::optional<SpotAnimationDefinition> definition =
            parser.parse(record);

        if (!definition.has_value()) {
            throw std::runtime_error(
                "Failed to parse spot-animation definition " +
                std::to_string(record.id)
            );
        }

        definitions_.push_back(
            std::move(*definition)
        );
    }
}

const SpotAnimationDefinition&
SpotAnimationRepository::get(
    std::uint16_t id
) const {
    const SpotAnimationDefinition* definition =
        find(id);

    if (definition == nullptr) {
        throw std::out_of_range(
            "Spot-animation definition does not exist"
        );
    }

    return *definition;
}

const SpotAnimationDefinition*
SpotAnimationRepository::find(
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

const std::vector<SpotAnimationDefinition>&
SpotAnimationRepository::list() const {
    return definitions_;
}

bool SpotAnimationRepository::contains(
    std::uint16_t id
) const {
    return find(id) != nullptr;
}

std::size_t SpotAnimationRepository::count() const {
    return definitions_.size();
}

}
