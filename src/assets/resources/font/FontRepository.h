#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "Font.h"
#include "FontDecoder.h"

#include "archive/Archive.h"
#include "cache/Store.h"

namespace eld::font {

class FontRepository {
public:
    FontRepository(
        eld::cache::Store store,
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
    static eld::archive::Archive loadArchive(
        const eld::cache::Store& store,
        std::uint16_t archiveId
    );

private:
    eld::archive::Archive archive_;
    FontDecoder decoder_;
};

}
