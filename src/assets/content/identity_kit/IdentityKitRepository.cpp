#include "IdentityKitRepository.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "IdentityKitDefinitionParser.h"

namespace eld::definition {

IdentityKitRepository::IdentityKitRepository(
    DefinitionTable table
)
    : table_(std::move(table)) {
    definitions_.reserve(
        table_.count()
    );

    IdentityKitDefinitionParser parser;

    for (
        const DefinitionRecord& record :
        table_.list()
    ) {
        std::optional<IdentityKitDefinition> definition =
            parser.parse(
                record.id,
                record.bytes
            );

        if (!definition.has_value()) {
            throw std::runtime_error(
                "Failed to parse identity-kit definition " +
                std::to_string(record.id)
            );
        }

        definitions_.push_back(
            std::move(*definition)
        );
    }
}

const IdentityKitDefinition& IdentityKitRepository::get(
    std::uint16_t id
) const {
    const IdentityKitDefinition* definition =
        find(id);

    if (definition == nullptr) {
        throw std::out_of_range(
            "Identity-kit definition does not exist"
        );
    }

    return *definition;
}

const IdentityKitDefinition* IdentityKitRepository::find(
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

const std::vector<IdentityKitDefinition>&
IdentityKitRepository::list() const {
    return definitions_;
}

bool IdentityKitRepository::contains(
    std::uint16_t id
) const {
    return find(id) != nullptr;
}

std::size_t IdentityKitRepository::count() const {
    return definitions_.size();
}

}
