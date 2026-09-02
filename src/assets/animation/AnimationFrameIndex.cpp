#include "AnimationFrameIndex.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace eld::animation {

AnimationFrameIndex::AnimationFrameIndex(
    const AnimationRepository& repository
) {
    for (
        const std::uint16_t archiveId :
        repository.listIds()
    ) {
        Animation animation =
            repository.get(archiveId);

        for (
            std::size_t frameIndex = 0;
            frameIndex < animation.asset.frames.size();
            ++frameIndex
        ) {
            const std::uint16_t frameId =
                animation.asset.frames[frameIndex].id;

            const auto [iterator, inserted] =
                frames_.emplace(
                    frameId,
                    FrameLocation{
                        archiveId,
                        frameIndex
                    }
                );

            if (!inserted) {
                throw std::runtime_error(
                    "Duplicate global animation frame id " +
                    std::to_string(frameId)
                );
            }

            (void) iterator;
        }

        archives_.emplace(
            archiveId,
            std::move(animation)
        );
    }
}

ResolvedAnimationFrame AnimationFrameIndex::resolve(
    std::uint16_t frameId
) const {
    const auto location =
        frames_.find(frameId);

    if (location == frames_.end()) {
        return {};
    }

    const auto archive =
        archives_.find(
            location->second.archiveId
        );

    if (archive == archives_.end()) {
        return {};
    }

    const Animation& animation =
        archive->second;

    if (
        location->second.frameIndex >=
        animation.asset.frames.size()
    ) {
        return {};
    }

    return {
        &animation.asset.frames[
            location->second.frameIndex
        ],
        &animation.asset.skeleton,
        location->second.archiveId
    };
}

bool AnimationFrameIndex::contains(
    std::uint16_t frameId
) const {
    return frames_.contains(frameId);
}

std::size_t AnimationFrameIndex::frameCount() const {
    return frames_.size();
}

std::size_t AnimationFrameIndex::archiveCount() const {
    return archives_.size();
}

}
