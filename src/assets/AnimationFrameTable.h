#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>

#include "Animation.h"

namespace eld::animation {

class AnimationRepository;

class AnimationFrameTable {
public:
    explicit AnimationFrameTable(
        const AnimationRepository& animations
    );

    std::optional<AnimationFrameView> find(
        std::uint16_t frameId
    ) const;

    bool contains(
        std::uint16_t frameId
    ) const;

private:
    struct FrameLocation {
        std::uint16_t animationId = 0;
        std::size_t frameIndex = 0;
    };

    void build() const;

    const AnimationRepository* repository_;

    mutable std::map<
        std::uint16_t,
        Animation
    > animations_;

    mutable std::map<
        std::uint16_t,
        FrameLocation
    > frames_;

    mutable bool built_ = false;
};

}
