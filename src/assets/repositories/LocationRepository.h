#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "Location.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "decoders/LocationDecoder.h"

namespace eld::location {

class LocationRepository {
public:
    explicit LocationRepository(
        const eld::cache::Cache& cache
    );

    Location get(
        std::uint16_t id
    ) const;

    std::optional<Location> find(
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
        "loc.dat";

    static constexpr std::string_view IndexFile =
        "loc.idx";

    eld::archive::Archive archive_;
    LocationDecoder decoder_;
};

}
