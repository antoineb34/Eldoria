#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "Animation.h"
#include "cache/Cache.h"
#include "decoders/AnimationDecoder.h"

namespace eld::animation {

class AnimationRepository {
public:
    explicit AnimationRepository(
        const eld::cache::Cache& cache
    );

    Animation get(
        std::uint16_t id
    ) const;

    std::optional<Animation> find(
        std::uint16_t id
    ) const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    static constexpr auto Index =
        eld::cache::IndexId::Animations;

    eld::cache::Store store_;
    AnimationDecoder decoder_;
};

}
