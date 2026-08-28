#pragma once

#include <cstddef>
#include <cstdint>
#include <map>

#include "Animation.h"
#include "AnimationRepository.h"

namespace eld::animation {

struct ResolvedAnimationFrame {
    const AnimationFrame* frame = nullptr;
    const Skeleton* skeleton = nullptr;
    std::uint16_t archiveId = 0;

    explicit operator bool() const {
        return frame != nullptr && skeleton != nullptr;
    }
};

class AnimationFrameIndex {
public:
    explicit AnimationFrameIndex(
        const AnimationRepository& repository
    );

    ResolvedAnimationFrame resolve(
        std::uint16_t frameId
    ) const;

    bool contains(
        std::uint16_t frameId
    ) const;

    std::size_t frameCount() const;
    std::size_t archiveCount() const;

private:
    struct FrameLocation {
        std::uint16_t archiveId = 0;
        std::size_t frameIndex = 0;
    };

    std::map<std::uint16_t, Animation> archives_;
    std::map<std::uint16_t, FrameLocation> frames_;
};

}
