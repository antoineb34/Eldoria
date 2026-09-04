#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "Sequence.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "decoders/SequenceDecoder.h"

namespace eld::sequence {

class SequenceRepository {
public:
    explicit SequenceRepository(
        const eld::cache::Cache& cache
    );

    Sequence get(
        std::uint16_t id
    ) const;

    std::optional<Sequence> find(
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
        "seq.dat";

    static constexpr std::string_view IndexFile =
        "seq.idx";

    eld::archive::Archive archive_;
    SequenceDecoder decoder_;
};

}
