#pragma once

#include <cstdint>
#include <filesystem>
#include <map>

#include "AnimationPresentation.h"
#include "definition/item/ItemDefinition.h"
#include "definition/npc/NpcDefinition.h"

namespace eld::animation::presentation {

class AnimationPresentationCatalog {
public:
    AnimationPresentationCatalog() = default;

    explicit AnimationPresentationCatalog(
        const std::filesystem::path& path
    );

    void load(
        const std::filesystem::path& path
    );

    NpcAnimationProfile resolveNpc(
        const eld::definition::NpcDefinition& definition
    ) const;

    ItemAnimationProfile resolveItem(
        const eld::definition::ItemDefinition& definition
    ) const;

private:
    enum class EntityKind : std::uint8_t {
        Npc,
        Item
    };

    struct Key {
        EntityKind kind = EntityKind::Npc;
        std::uint16_t id = 0;
        AnimationAction action = AnimationAction::Idle;

        bool operator<(
            const Key& other
        ) const {
            if (kind != other.kind) {
                return kind < other.kind;
            }

            if (id != other.id) {
                return id < other.id;
            }

            return action < other.action;
        }
    };

    std::map<Key, AnimationBinding> authored_;
};

}
