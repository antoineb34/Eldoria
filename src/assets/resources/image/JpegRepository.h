#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "Image.h"
#include "JpegDecoder.h"

#include "archive/Archive.h"
#include "cache/Store.h"

namespace eld::image {

class JpegRepository {
public:
    JpegRepository(
        eld::cache::Store store,
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
    static eld::archive::Archive loadArchive(
        const eld::cache::Store& store,
        std::uint16_t archiveId
    );

private:
    eld::archive::Archive archive_;
    JpegDecoder decoder_;
};

}
