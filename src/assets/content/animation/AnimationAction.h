#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace eld::animation::presentation {

enum class AnimationAction : std::uint8_t {
    Idle,
    Walk,
    TurnAround,
    TurnLeft,
    TurnRight,
    Attack,
    Defend,
    Death,
    SpecialAttack,
    Cast,
    Use,
    Emote
};

inline std::string_view toString(
    AnimationAction action
) {
    switch (action) {
        case AnimationAction::Idle:          return "Idle";
        case AnimationAction::Walk:          return "Walk";
        case AnimationAction::TurnAround:    return "Turn around";
        case AnimationAction::TurnLeft:      return "Turn left";
        case AnimationAction::TurnRight:     return "Turn right";
        case AnimationAction::Attack:        return "Attack";
        case AnimationAction::Defend:        return "Defend";
        case AnimationAction::Death:         return "Death";
        case AnimationAction::SpecialAttack: return "Special attack";
        case AnimationAction::Cast:          return "Cast";
        case AnimationAction::Use:           return "Use";
        case AnimationAction::Emote:         return "Emote";
    }

    return "Unknown";
}

inline std::optional<AnimationAction> animationActionFromString(
    std::string_view value
) {
    if (value == "idle") return AnimationAction::Idle;
    if (value == "walk") return AnimationAction::Walk;
    if (value == "turn_around") return AnimationAction::TurnAround;
    if (value == "turn_left") return AnimationAction::TurnLeft;
    if (value == "turn_right") return AnimationAction::TurnRight;
    if (value == "attack") return AnimationAction::Attack;
    if (value == "defend") return AnimationAction::Defend;
    if (value == "death") return AnimationAction::Death;
    if (value == "special_attack") return AnimationAction::SpecialAttack;
    if (value == "cast") return AnimationAction::Cast;
    if (value == "use") return AnimationAction::Use;
    if (value == "emote") return AnimationAction::Emote;

    return std::nullopt;
}

}
