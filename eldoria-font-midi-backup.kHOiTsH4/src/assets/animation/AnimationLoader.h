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

class AnimationLoader {
public:
    explicit AnimationLoader(
        const eld::cache::Cache& cache
    );

    AnimationLoader(const AnimationLoader&) = delete;
    AnimationLoader& operator=(const AnimationLoader&) = delete;
    AnimationLoader(AnimationLoader&&) = delete;
    AnimationLoader& operator=(AnimationLoader&&) = delete;

    const AnimationData& data(
        std::uint16_t id
    ) const;

    std::optional<AnimationData> find(
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

    AnimationData loadData(
        std::uint16_t id
    ) const;

    eld::cache::Store store_;
    AnimationDecoder decoder_;

    mutable std::map<
        std::uint16_t,
        AnimationData
    > dataCache_;

};

}
