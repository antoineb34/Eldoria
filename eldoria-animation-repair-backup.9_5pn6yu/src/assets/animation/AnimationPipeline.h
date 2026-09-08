#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <span>
#include <vector>

#include "animation/AnimationData.h"
#include "animation/AnimationDecoder.h"
#include "cache/Cache.h"

namespace eld::animation {

class AnimationPipeline {
public:
    explicit AnimationPipeline(
        const eld::cache::Cache& cache
    );

    const AnimationData& data(
        std::uint16_t id
    ) const;

    std::optional<AnimationData> find(
        std::uint16_t id
    ) const;

    std::optional<AnimationFrameView> findFrame(
        std::uint16_t frameId
    ) const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    bool containsFrame(
        std::uint16_t frameId
    ) const;

    std::size_t count() const;

private:
    struct FrameLocation {
        std::uint16_t animationId = 0;
        std::size_t frameIndex = 0;
    };

    static constexpr auto Index =
        eld::cache::IndexId::Animations;

    AnimationData loadData(
        std::uint16_t id
    ) const;

    void buildFrameIndex() const;

    eld::cache::Store store_;
    AnimationDecoder decoder_;

    mutable std::map<
        std::uint16_t,
        AnimationData
    > dataCache_;

    mutable std::map<
        std::uint16_t,
        FrameLocation
    > frameIndex_;

    mutable bool frameIndexBuilt_ = false;
};

}
