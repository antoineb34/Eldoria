#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

#include "Widget.h"
#include "cache/Cache.h"
#include "decoders/WidgetDecoder.h"

namespace eld::interface {

class WidgetRepository {
public:
    explicit WidgetRepository(
        const eld::cache::Cache& cache
    );

    const Widget& get(
        std::uint16_t id
    ) const;

    const Widget* find(
        std::uint16_t id
    ) const;

    const std::vector<Widget>&
    list() const;

    std::vector<std::uint16_t>
    listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    static constexpr auto Index =
        eld::cache::IndexId::Config;

    static constexpr std::uint16_t ArchiveId =
        3;

    static constexpr std::string_view DataFile =
        "data";

    std::vector<Widget> widgets_;
    WidgetDecoder decoder_;
};

}
