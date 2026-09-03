#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <span>
#include <vector>

#include "Animation.h"
#include "AnimationDecoder.h"
#include "cache/Store.h"

namespace eld::animation {


class AnimationRepository {
public:
    explicit AnimationRepository(
        eld::cache::Store store
    );

    Animation get(
        std::uint16_t id
    ) const;

    std::optional<Animation> find(
        std::uint16_t id
    ) const;

    std::optional<AnimationFrameView> findFrame(
        std::uint16_t frameId
    ) const;

    bool containsFrame(
        std::uint16_t frameId
    ) const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    struct FrameLocation {
        std::uint16_t animationId = 0;
        std::size_t frameIndex = 0;
    };

    const Animation& load(
        std::uint16_t id
    ) const;

    void ensureFrameIndex() const;

    eld::cache::Store store_;
    AnimationDecoder decoder_;

    mutable std::map<std::uint16_t, Animation> animations_;
    mutable std::map<std::uint16_t, FrameLocation> frames_;
    mutable bool frameIndexBuilt_ = false;
};

}
