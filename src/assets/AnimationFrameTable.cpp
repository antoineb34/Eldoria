#include "AnimationFrameTable.h"

#include <stdexcept>
#include <string>
#include <utility>

#include "repositories/AnimationRepository.h"

namespace eld::animation {

AnimationFrameTable::AnimationFrameTable(
    const AnimationRepository& animations
)
    : repository_(&animations) {
}


void AnimationFrameTable::build() const {
    if (built_) {
        return;
    }

    std::map<
        std::uint16_t,
        Animation
    > animations;

    std::map<
        std::uint16_t,
        FrameLocation
    > frames;

    for (
        const std::uint16_t animationId :
        repository_->listIds()
    ) {
        Animation animation =
            repository_->get(
                animationId
            );

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

            (void) entry;
        }

        animations.emplace(
            animationId,
            std::move(animation)
        );
    }

    animations_ =
        std::move(animations);

    frames_ =
        std::move(frames);

    built_ = true;
}


std::optional<AnimationFrameView>
AnimationFrameTable::find(
    std::uint16_t frameId
) const {
    build();

    const auto location =
        frames_.find(frameId);

    if (location == frames_.end()) {
        return std::nullopt;
    }

    const auto animation =
        animations_.find(
            location->second.animationId
        );

    if (animation == animations_.end()) {
        return std::nullopt;
    }

    if (
        location->second.frameIndex >=
        animation->second.frames.size()
    ) {
        return std::nullopt;
    }

    return AnimationFrameView{
        animation->second.frames[
            location->second.frameIndex
        ],
        animation->second.skeleton
    };
}


bool AnimationFrameTable::contains(
    std::uint16_t frameId
) const {
    build();

    return frames_.contains(
        frameId
    );
}

}
