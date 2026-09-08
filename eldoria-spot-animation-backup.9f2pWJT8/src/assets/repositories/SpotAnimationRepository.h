#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "SpotAnimation.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "decoders/SpotAnimationDecoder.h"

namespace eld::spot_animation {

class SpotAnimationRepository {
public:
    explicit SpotAnimationRepository(
        const eld::cache::Cache& cache
    );

    SpotAnimation get(
        std::uint16_t id
    ) const;

    std::optional<SpotAnimation> find(
        std::uint16_t id
    ) const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    static constexpr auto Index =
        eld::cache::IndexId::Config;

    static constexpr std::uint16_t ArchiveId =
        2;

    static constexpr std::string_view DataFile =
        "spotanim.dat";

    static constexpr std::string_view IndexFile =
        "spotanim.idx";

    eld::archive::Archive archive_;
    SpotAnimationDecoder decoder_;
};

}
