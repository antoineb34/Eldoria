#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "Image.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "decoders/ImageDecoder.h"

namespace eld::image {

class ImageRepository {
public:
    ImageRepository(
        const eld::cache::Cache& cache,
        std::uint16_t archiveId
    );

    Image get(
        std::string_view fileName
    ) const;

    std::optional<Image> find(
        std::string_view fileName
    ) const;

    bool contains(
        std::string_view fileName
    ) const;

private:
    static constexpr auto Index =
        eld::cache::IndexId::Config;

    eld::archive::Archive archive_;
    ImageDecoder decoder_;
};

}
