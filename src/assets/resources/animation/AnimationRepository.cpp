#include "AnimationRepository.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

namespace eld::animation {

AnimationRepository::AnimationRepository(
    eld::cache::Store store
)
    : store_(std::move(store)) {
}

const Animation& AnimationRepository::load(
    std::uint16_t id
) const {
    const auto cached =
        animations_.find(id);

    if (cached != animations_.end()) {
        return cached->second;
    }

    eld::cache::File file =
        store_.get(id);

    try {
        Animation animation =
            decoder_.decode(
                file.getBytes()
            );

        animation.id = id;

        const auto [entry, inserted] =
            animations_.emplace(
                id,
                std::move(animation)
            );

        (void) inserted;
        return entry->second;
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode animation " +
            std::to_string(id) +
            ": " +
            error.what()
        );
    }
}

Animation AnimationRepository::get(
    std::uint16_t id
) const {
    return load(id);
}

std::optional<Animation>
AnimationRepository::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return get(id);
}

void AnimationRepository::ensureFrameIndex() const {
    if (frameIndexBuilt_) {
        return;
    }

    std::map<std::uint16_t, FrameLocation>
        frameIndex;

    for (
        const std::uint16_t animationId :
        listIds()
    ) {
        const Animation& animation =
            load(animationId);

        for (
            std::size_t frameIndexValue = 0;
            frameIndexValue < animation.frames.size();
            ++frameIndexValue
        ) {
            const std::uint16_t frameId =
                animation.frames[
                    frameIndexValue
                ].id;

            const auto [entry, inserted] =
                frameIndex.emplace(
                    frameId,
                    FrameLocation{
                        animationId,
                        frameIndexValue
                    }
                );

            if (!inserted) {
                throw std::runtime_error(
                    "Duplicate global animation frame id " +
                    std::to_string(frameId)
                );
            }

            (void) entry;
        }
    }

    frames_ = std::move(frameIndex);
    frameIndexBuilt_ = true;
}

std::optional<AnimationFrameView>
AnimationRepository::findFrame(
    std::uint16_t frameId
) const {
    ensureFrameIndex();

    const auto location =
        frames_.find(frameId);

    if (location == frames_.end()) {
        return std::nullopt;
    }

    const Animation& animation =
        load(
            location->second.animationId
        );

    if (
        location->second.frameIndex >=
        animation.frames.size()
    ) {
        return std::nullopt;
    }

    return AnimationFrameView{
        animation.frames[
            location->second.frameIndex
        ],
        animation.skeleton
    };
}

bool AnimationRepository::containsFrame(
    std::uint16_t frameId
) const {
    ensureFrameIndex();
    return frames_.contains(frameId);
}

std::vector<std::uint16_t>
AnimationRepository::listIds() const {
    const std::vector<eld::cache::FileEntry> entries =
        store_.list();

    std::vector<std::uint16_t> ids;
    ids.reserve(entries.size());

    for (const eld::cache::FileEntry& entry : entries) {
        ids.push_back(entry.fileId);
    }

    return ids;
}

bool AnimationRepository::contains(
    std::uint16_t id
) const {
    return store_.contains(id);
}

std::size_t AnimationRepository::count() const {
    return store_.count();
}

}
