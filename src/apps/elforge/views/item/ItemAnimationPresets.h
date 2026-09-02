#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "animation/AnimationPresentation.h"

namespace eld::definition {
struct ItemDefinition;
}

namespace eld::elforge {

class ItemAnimationPresets {
public:
    static std::vector<
        eld::animation::presentation::AnimationBinding
    > actions(
        const eld::definition::ItemDefinition& definition
    );

    static std::optional<std::uint16_t> defendSequence(
        const eld::definition::ItemDefinition& definition
    );
};

}
