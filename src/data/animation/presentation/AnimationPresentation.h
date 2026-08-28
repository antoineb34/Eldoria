#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "AnimationAction.h"

namespace eld::animation::presentation {

struct AnimationEffectBinding {
    std::uint16_t spotAnimationId = 0;
    bool projectile = false;

    // Client-side presentation timing only. Gameplay/damage timing remains
    // authoritative elsewhere.
    std::uint32_t delayMilliseconds = 0;
    std::uint32_t durationMilliseconds = 700;
};

struct AnimationBinding {
    AnimationAction action = AnimationAction::Idle;
    std::optional<std::uint16_t> sequenceId;
    std::vector<AnimationEffectBinding> effects;
};

struct NpcAnimationProfile {
    std::uint16_t npcId = 0;
    std::vector<AnimationBinding> bindings;
};

struct ItemAnimationProfile {
    std::uint16_t itemId = 0;
    std::vector<AnimationBinding> bindings;
};

}
