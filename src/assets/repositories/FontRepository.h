#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "Font.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "decoders/FontDecoder.h"

namespace eld::font {

class FontRepository {
public:
    FontRepository(
        const eld::cache::Cache& cache,
        std::uint16_t archiveId
    );

    Font get(
        std::string_view name
    ) const;

    std::optional<Font> find(
        std::string_view name
    ) const;

    bool contains(
        std::string_view name
    ) const;

private:
    static constexpr auto Index =
        eld::cache::IndexId::Config;

    static constexpr std::string_view IndexFile =
        "index.dat";

    eld::archive::Archive archive_;
    FontDecoder decoder_;
};

}
