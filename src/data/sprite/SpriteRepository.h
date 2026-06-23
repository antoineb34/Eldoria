#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "Sprite.h"

#include "archive/Archive.h"
#include "cache/Store.h"
#include "image/IndexedImageDecoder.h"
#include "image/IndexedImageFileParser.h"

namespace eld::sprite {

class SpriteRepository {
public:
    SpriteRepository(
        eld::cache::Store store,
        std::uint16_t archiveId
    );

    Sprite get(
        std::string_view groupName,
        std::uint16_t frameId = 0
    ) const;

    std::optional<Sprite> find(
        std::string_view groupName,
        std::uint16_t frameId = 0
    ) const;

    eld::image::IndexedImageFile getFile(
        std::string_view groupName,
        std::uint16_t frameId = 0
    ) const;

    eld::image::Image getImage(
        std::string_view groupName,
        std::uint16_t frameId = 0
    ) const;

    bool contains(
        std::string_view groupName,
        std::uint16_t frameId = 0
    ) const;

private:
    static eld::archive::Archive loadArchive(
        const eld::cache::Store& store,
        std::uint16_t archiveId
    );

    const eld::archive::ArchiveFile& getIndexFile() const;

    const eld::archive::ArchiveFile& getDataFile(
        std::string_view groupName
    ) const;

private:
    eld::archive::Archive archive_;

    eld::image::IndexedImageFileParser parser_;
    eld::image::IndexedImageDecoder decoder_;
};

}
