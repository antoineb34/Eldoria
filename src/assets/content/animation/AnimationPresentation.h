#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "AnimationAction.h"

namespace eld::animation::presentation {

struct AnimationEffectBinding {
    std::uint16_t spotAnimationId = 0;
    bool projectile = false;
    bool target = false;

    // Client-side presentation timing only. Gameplay/damage timing remains
    // authoritative elsewhere.
    std::uint32_t delayMilliseconds = 0;
    std::uint32_t durationMilliseconds = 700;

    // RuneTek projectile presentation parameters. The classic client received
    // these from the server for every projectile; heights are quarter-units
    // and are multiplied by four when placed in scene/model coordinates.
    std::uint16_t projectileStartHeight = 40;
    std::uint16_t projectileEndHeight = 36;
    std::uint16_t projectileSlope = 15;
    std::uint16_t projectileStartDistance = 11;
};

struct AnimationBinding {
    AnimationAction action = AnimationAction::Idle;
    std::string variant;
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
