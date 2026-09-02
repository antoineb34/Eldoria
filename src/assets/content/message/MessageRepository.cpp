#include "MessageRepository.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "MessageDefinitionParser.h"

namespace eld::definition {

MessageRepository::MessageRepository(
    DefinitionTable table
)
    : table_(std::move(table)) {
    definitions_.reserve(
        table_.count()
    );

    MessageDefinitionParser parser;

    for (
        const DefinitionRecord& record :
        table_.list()
    ) {
        std::optional<MessageDefinition> definition =
            parser.parse(record);

        if (!definition.has_value()) {
            throw std::runtime_error(
                "Failed to parse message definition " +
                std::to_string(record.id)
            );
        }

        definitions_.push_back(
            std::move(*definition)
        );
    }
}

const MessageDefinition& MessageRepository::get(
    std::uint16_t id
) const {
    const MessageDefinition* definition =
        find(id);

    if (definition == nullptr) {
        throw std::out_of_range(
            "Message definition does not exist"
        );
    }

    return *definition;
}

const MessageDefinition* MessageRepository::find(
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

const std::vector<MessageDefinition>&
MessageRepository::list() const {
    return definitions_;
}

bool MessageRepository::contains(
    std::uint16_t id
) const {
    return find(id) != nullptr;
}

std::size_t MessageRepository::count() const {
    return definitions_.size();
}

}
