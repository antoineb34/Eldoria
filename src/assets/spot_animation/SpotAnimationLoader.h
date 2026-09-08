#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string_view>
#include <vector>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "model/ModelLoader.h"
#include "sequence/SequenceLoader.h"
#include "spot_animation/SpotAnimationAssembler.h"
#include "spot_animation/SpotAnimationData.h"
#include "spot_animation/SpotAnimationDecoder.h"
#include "spot_animation/SpotAnimationResource.h"

namespace eld::spot_animation {

class SpotAnimationLoader {
public:
    explicit SpotAnimationLoader(
        const eld::cache::Cache& cache
    );

    SpotAnimationLoader(
        const eld::cache::Cache& cache,
        const eld::model::ModelLoader& models,
        const eld::sequence::SequenceLoader& sequences
    );

    const SpotAnimationData& data(
        std::uint16_t id
    ) const;

    std::optional<SpotAnimationData> find(
        std::uint16_t id
    ) const;

    const SpotAnimationResource& resource(
        std::uint16_t id
    ) const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    static constexpr auto Index = eld::cache::IndexId::Config;
    static constexpr std::uint16_t ArchiveId = 2;
    static constexpr std::string_view DataFile = "spotanim.dat";
    static constexpr std::string_view IndexFile = "spotanim.idx";

    SpotAnimationData loadData(
        std::uint16_t id
    ) const;

    eld::archive::Archive archive_;
    SpotAnimationDecoder decoder_;
    SpotAnimationAssembler assembler_;
    mutable std::map<
        std::uint16_t,
        SpotAnimationData
    > dataCache_;

    mutable std::map<
        std::uint16_t,
        SpotAnimationResource
    > resourceCache_;
};

}
