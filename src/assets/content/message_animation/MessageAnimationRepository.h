#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "definition/DefinitionTable.h"
#include "MessageAnimationDefinition.h"

namespace eld::definition {

class MessageAnimationRepository {
public:
    explicit MessageAnimationRepository(
        DefinitionTable table
    );

    const MessageAnimationDefinition& get(
        std::uint16_t id
    ) const;

    const MessageAnimationDefinition* find(
        std::uint16_t id
    ) const;

    const std::vector<MessageAnimationDefinition>&
    list() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    DefinitionTable table_;
    std::vector<MessageAnimationDefinition> definitions_;
};

}
