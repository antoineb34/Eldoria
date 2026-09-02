#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

#include "Animation.h"
#include "AnimationDecoder.h"
#include "AnimationFileParser.h"
#include "cache/Store.h"

namespace eld::animation {

using AnimationPredicate =
    std::function<bool(const Animation&)>;

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

    AnimationFile getFile(
        std::uint16_t id
    ) const;

    AnimationAsset getAsset(
        std::uint16_t id
    ) const;

    std::vector<std::uint16_t> listIds() const;

    std::vector<std::uint16_t> filterIds(
        const AnimationPredicate& predicate
    ) const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

    std::size_t count(
        const AnimationPredicate& predicate
    ) const;

private:
    eld::cache::Store store_;
    AnimationFileParser parser_;
    AnimationDecoder decoder_;
};

}
