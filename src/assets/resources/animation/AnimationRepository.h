#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
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

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    eld::cache::Store store_;
    AnimationDecoder decoder_;
};

}
