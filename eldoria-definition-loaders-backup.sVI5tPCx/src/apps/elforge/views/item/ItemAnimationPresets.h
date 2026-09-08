#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "animation/AnimationPresentation.h"

namespace eld::item {
struct Item;
}

namespace eld::elforge {

class ItemAnimationPresets {
public:
    static std::vector<
        eld::animation::presentation::AnimationBinding
    > actions(
        const eld::item::Item& definition
    );

    static std::optional<std::uint16_t> defendSequence(
        const eld::item::Item& definition
    );
};

}
