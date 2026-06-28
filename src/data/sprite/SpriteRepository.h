#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

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

    Sprite get(
        std::uint16_t fileId,
        std::uint16_t frameId = 0
    ) const;

    std::optional<Sprite> find(
        std::string_view groupName,
        std::uint16_t frameId = 0
    ) const;

    std::optional<Sprite> find(
        std::uint16_t fileId,
        std::uint16_t frameId = 0
    ) const;

    eld::image::IndexedImageFile getFile(
        std::string_view groupName,
        std::uint16_t frameId = 0
    ) const;

    eld::image::IndexedImageFile getFile(
        std::uint16_t fileId,
        std::uint16_t frameId = 0
    ) const;

    bool contains(
        std::string_view groupName,
        std::uint16_t frameId = 0
    ) const;

    bool contains(
        std::uint16_t fileId,
        std::uint16_t frameId = 0
    ) const;

    std::vector<std::uint16_t> listFrameIds(
        std::string_view groupName
    ) const;

    std::vector<std::uint16_t> listFrameIds(
        std::uint16_t fileId
    ) const;

    std::size_t countFrames(
        std::string_view groupName
    ) const;

    std::size_t countFrames(
        std::uint16_t fileId
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

    const eld::archive::ArchiveFile& getDataFile(
        std::uint16_t fileId
    ) const;

    eld::image::IndexedImageFile parseFile(
        const eld::archive::ArchiveFile& dataFile,
        std::uint16_t frameId
    ) const;

    Sprite decodeSprite(
        const eld::archive::ArchiveFile& dataFile,
        std::string_view groupName,
        std::uint16_t frameId
    ) const;

private:
    eld::archive::Archive archive_;
    eld::image::IndexedImageFileParser parser_;
    eld::image::IndexedImageDecoder decoder_;
};

}
