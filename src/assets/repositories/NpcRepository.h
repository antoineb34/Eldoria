#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "Npc.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "decoders/NpcDecoder.h"

namespace eld::npc {

class NpcRepository {
public:
    explicit NpcRepository(
        const eld::cache::Cache& cache
    );

    Npc get(
        std::uint16_t id
    ) const;

    std::optional<Npc> find(
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
        "npc.dat";

    static constexpr std::string_view IndexFile =
        "npc.idx";

    eld::archive::Archive archive_;
    NpcDecoder decoder_;
};

}
