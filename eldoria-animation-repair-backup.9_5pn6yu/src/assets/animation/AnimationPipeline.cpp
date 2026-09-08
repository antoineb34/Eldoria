#include "animation/AnimationPipeline.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

namespace eld::animation {

AnimationPipeline::AnimationPipeline(
    const eld::cache::Cache& cache
)
    : store_(
          cache.open(Index)
      ) {
}


const AnimationData& AnimationPipeline::data(
    std::uint16_t id
) const {
    const auto existing =
        dataCache_.find(id);

    if (existing != dataCache_.end()) {
        return existing->second;
    }

    const auto inserted =
        dataCache_.emplace(
            id,
            loadData(id)
        );

    return inserted.first->second;
}


AnimationData AnimationPipeline::loadData(
    std::uint16_t id
) const {
    const eld::cache::File file =
        store_.get(id);

    try {
        AnimationData animation =
            decoder_.decode(
                file.getBytes()
            );

        animation.id = id;
        return animation;
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


std::optional<AnimationData>
AnimationPipeline::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return data(id);
}


void AnimationPipeline::buildFrameIndex() const {
    if (frameIndexBuilt_) {
        return;
    }

    std::map<
        std::uint16_t,
        FrameLocation
    > frameIndex;

    for (
        const std::uint16_t animationId :
        listIds()
    ) {
        const AnimationData& animation =
            data(animationId);

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
                    "Duplicate animation frame " +
                    std::to_string(frameId)
                );
            }

            (void) entry;
        }
    }

    frameIndex_ = std::move(frameIndex);
    frameIndexBuilt_ = true;
}


std::optional<AnimationFrameView>
AnimationPipeline::findFrame(
    std::uint16_t frameId
) const {
    buildFrameIndex();

    const auto location =
        frameIndex_.find(frameId);

    if (location == frameIndex_.end()) {
        return std::nullopt;
    }

    const AnimationData& animation =
        data(location->second.animationId);

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


std::vector<std::uint16_t>
AnimationPipeline::listIds() const {
    const std::vector<eld::cache::FileEntry> entries =
        store_.list();

    std::vector<std::uint16_t> ids;
    ids.reserve(entries.size());

    for (
        const eld::cache::FileEntry& entry :
        entries
    ) {
        ids.push_back(entry.fileId);
    }

    return ids;
}


bool AnimationPipeline::contains(
    std::uint16_t id
) const {
    return store_.contains(id);
}


bool AnimationPipeline::containsFrame(
    std::uint16_t frameId
) const {
    buildFrameIndex();
    return frameIndex_.contains(frameId);
}


std::size_t AnimationPipeline::count() const {
    return store_.count();
}

}
