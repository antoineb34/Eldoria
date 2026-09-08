#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "Varp.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "decoders/VarpDecoder.h"

namespace eld::varp {

class VarpRepository {
public:
    explicit VarpRepository(
        const eld::cache::Cache& cache
    );

    Varp get(
        std::uint16_t id
    ) const;

    std::optional<Varp> find(
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
        "varp.dat";

    static constexpr std::string_view IndexFile =
        "varp.idx";

    eld::archive::Archive archive_;
    VarpDecoder decoder_;
};

}
