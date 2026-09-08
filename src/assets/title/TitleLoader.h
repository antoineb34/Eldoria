#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "image/ImageData.h"
#include "image/ImageDecoder.h"

namespace eld::title {

class TitleLoader {
public:
    explicit TitleLoader(
        const eld::cache::Cache& cache
    );

    const eld::image::ImageData& data(
        std::string_view name
    ) const;

    std::optional<eld::image::ImageData> find(
        std::string_view name
    ) const;

    bool contains(
        std::string_view name
    ) const;

private:
    static constexpr auto Index =
        eld::cache::IndexId::Config;

    static constexpr std::uint16_t ArchiveId = 1;

    eld::archive::Archive archive_;
    eld::image::ImageDecoder decoder_;

    mutable std::map<
        std::string,
        eld::image::ImageData
    > dataCache_;
};

}
