#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "Varbit.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "decoders/VarbitDecoder.h"

namespace eld::varbit {

class VarbitRepository {
public:
    explicit VarbitRepository(
        const eld::cache::Cache& cache
    );

    Varbit get(
        std::uint16_t id
    ) const;

    std::optional<Varbit> find(
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
        "varbit.dat";

    static constexpr std::string_view IndexFile =
        "varbit.idx";

    eld::archive::Archive archive_;
    VarbitDecoder decoder_;
};

}
