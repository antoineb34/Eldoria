#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string_view>
#include <vector>

#include "animation/AnimationFrameTable.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "sequence/SequenceAssembler.h"
#include "sequence/SequenceData.h"
#include "sequence/SequenceDecoder.h"
#include "sequence/SequenceResource.h"

namespace eld::sequence {

class SequencePipeline {
public:
    explicit SequencePipeline(
        const eld::cache::Cache& cache
    );

    SequencePipeline(
        const eld::cache::Cache& cache,
        const eld::animation::AnimationFrameTable& frames
    );

    const SequenceData& data(
        std::uint16_t id
    ) const;

    const SequenceResource& resource(
        std::uint16_t id
    ) const;

    std::optional<SequenceResource> find(
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

    static constexpr std::uint16_t ArchiveId = 2;

    static constexpr std::string_view DataFile =
        "seq.dat";

    static constexpr std::string_view IndexFile =
        "seq.idx";

    SequenceData loadData(
        std::uint16_t id
    ) const;

    eld::archive::Archive archive_;

    SequenceDecoder decoder_;
    SequenceAssembler assembler_;

    mutable std::map<
        std::uint16_t,
        SequenceData
    > dataCache_;

    mutable std::map<
        std::uint16_t,
        SequenceResource
    > resourceCache_;
};

}
