#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "Item.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "decoders/ItemDecoder.h"

namespace eld::item {

class ItemRepository {
public:
    explicit ItemRepository(
        const eld::cache::Cache& cache
    );

    Item get(
        std::uint16_t id
    ) const;

    std::optional<Item> find(
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
        "obj.dat";

    static constexpr std::string_view IndexFile =
        "obj.idx";

    eld::archive::Archive archive_;
    ItemDecoder decoder_;
};

}
