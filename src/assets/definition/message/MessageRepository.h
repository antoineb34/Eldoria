#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "../DefinitionTable.h"
#include "MessageDefinition.h"

namespace eld::definition {

class MessageRepository {
public:
    explicit MessageRepository(
        DefinitionTable table
    );

    const MessageDefinition& get(
        std::uint16_t id
    ) const;

    const MessageDefinition* find(
        std::uint16_t id
    ) const;

    const std::vector<MessageDefinition>&
    list() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    DefinitionTable table_;
    std::vector<MessageDefinition> definitions_;
};

}
