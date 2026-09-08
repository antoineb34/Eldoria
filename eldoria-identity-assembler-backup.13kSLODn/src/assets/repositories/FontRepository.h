#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "Font.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "decoders/FontDecoder.h"

namespace eld::font {

class FontRepository {
public:
    explicit FontRepository(
        const eld::cache::Cache& cache
    );

    Font get(
        std::uint16_t id
    ) const;

    std::optional<Font> find(
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
        1;

    static constexpr std::string_view IndexFile =
        "index.dat";

    eld::archive::Archive archive_;
    FontDecoder decoder_;
};

}
