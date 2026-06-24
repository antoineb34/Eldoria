#include "MessageAnimationRepository.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "MessageAnimationDefinitionParser.h"

namespace eld::definition {

MessageAnimationRepository::MessageAnimationRepository(
    DefinitionTable table
)
    : table_(std::move(table)) {
    definitions_.reserve(
        table_.count()
    );

    MessageAnimationDefinitionParser parser;

    for (
        const DefinitionRecord& record :
        table_.list()
    ) {
        std::optional<MessageAnimationDefinition> definition =
            parser.parse(record);

        if (!definition.has_value()) {
            throw std::runtime_error(
                "Failed to parse message animation definition " +
                std::to_string(record.id)
            );
        }

        definitions_.push_back(
            std::move(*definition)
        );
    }
}

const MessageAnimationDefinition& MessageAnimationRepository::get(
    std::uint16_t id
) const {
    const MessageAnimationDefinition* definition =
        find(id);

    if (definition == nullptr) {
        throw std::out_of_range(
            "MessageAnimation definition does not exist"
        );
    }

    return *definition;
}

const MessageAnimationDefinition* MessageAnimationRepository::find(
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

const std::vector<MessageAnimationDefinition>&
MessageAnimationRepository::list() const {
    return definitions_;
}

bool MessageAnimationRepository::contains(
    std::uint16_t id
) const {
    return find(id) != nullptr;
}

std::size_t MessageAnimationRepository::count() const {
    return definitions_.size();
}

}
