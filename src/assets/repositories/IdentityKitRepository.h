#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "IdentityKit.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "decoders/IdentityKitDecoder.h"

namespace eld::identity_kit {

class IdentityKitRepository {
public:
    explicit IdentityKitRepository(
        const eld::cache::Cache& cache
    );

    IdentityKit get(
        std::uint16_t id
    ) const;

    std::optional<IdentityKit> find(
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
        "idk.dat";

    static constexpr std::string_view IndexFile =
        "idk.idx";

    eld::archive::Archive archive_;
    IdentityKitDecoder decoder_;
};

}
