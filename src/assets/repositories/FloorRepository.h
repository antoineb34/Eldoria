#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "Floor.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "decoders/FloorDecoder.h"

namespace eld::floor {

class FloorRepository {
public:
    explicit FloorRepository(
        const eld::cache::Cache& cache
    );

    Floor get(
        std::uint16_t id
    ) const;

    std::optional<Floor> find(
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
        "flo.dat";

    static constexpr std::string_view IndexFile =
        "flo.idx";

    eld::archive::Archive archive_;
    FloorDecoder decoder_;
};

}
