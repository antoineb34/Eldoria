#include "animation/AnimationFrameTable.h"

#include <stdexcept>
#include <string>
#include <utility>

#include "animation/AnimationLoader.h"

namespace eld::animation {

AnimationFrameTable::AnimationFrameTable(
    const AnimationLoader& animations
)
    : animations_(&animations) {
}


void AnimationFrameTable::build() const {
    if (built_) {
        return;
    }

    std::map<
        std::uint16_t,
        FrameLocation
    > frames;

    for (
        const std::uint16_t animationId :
        animations_->listIds()
    ) {
        const AnimationData& animation =
            animations_->data(animationId);

        for (
            std::size_t frameIndex = 0;
            frameIndex < animation.frames.size();
            ++frameIndex
        ) {
            const std::uint16_t frameId =
                animation.frames[
                    frameIndex
                ].id;

            const auto [entry, inserted] =
                frames.emplace(
                    frameId,
                    FrameLocation{
                        animationId,
                        frameIndex
                    }
                );

            if (!inserted) {
                throw std::runtime_error(
                    "Duplicate animation frame " +
                    std::to_string(frameId)
                );
            }
        }
    }

    frames_ = std::move(frames);
    built_ = true;
}


std::optional<AnimationFrameResource>
AnimationFrameTable::find(
    std::uint16_t frameId
) const {
    build();

    const auto location =
        frames_.find(frameId);

    if (location == frames_.end()) {
        return std::nullopt;
    }

    const AnimationData& animation =
        animations_->data(
            location->second.animationId
        );

    if (
        location->second.frameIndex >=
        animation.frames.size()
    ) {
        return std::nullopt;
    }

    return AnimationFrameResource{
        animation.frames[
            location->second.frameIndex
        ],
        animation.skeleton
    };
}


bool AnimationFrameTable::contains(
    std::uint16_t frameId
) const {
    build();
    return frames_.contains(frameId);
}

}
