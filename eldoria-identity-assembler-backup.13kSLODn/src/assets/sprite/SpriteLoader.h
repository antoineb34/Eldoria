#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "image/ImageData.h"
#include "image/ImageDecoder.h"
#include "sprite/SpriteAssembler.h"
#include "sprite/SpriteResource.h"

namespace eld::sprite {

enum class SpriteArchive : std::uint16_t {
    Title = 1,
    Media = 4
};


class SpriteLoader {
public:
    SpriteLoader(
        const eld::cache::Cache& cache,
        SpriteArchive archive
    );

    const eld::image::ImageData& data(
        std::string_view groupName,
        std::uint16_t frameId = 0
    ) const;

    const eld::image::ImageData& data(
        std::uint16_t fileId,
        std::uint16_t frameId = 0
    ) const;

    const SpriteResource& resource(
        std::string_view groupName,
        std::uint16_t frameId = 0
    ) const;

    const SpriteResource& resource(
        std::uint16_t fileId,
        std::uint16_t frameId = 0
    ) const;

    std::optional<SpriteResource> find(
        std::string_view groupName,
        std::uint16_t frameId = 0
    ) const;

    std::optional<SpriteResource> find(
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
    using Key =
        std::pair<std::string, std::uint16_t>;

    static constexpr auto Index =
        eld::cache::IndexId::Config;

    const eld::archive::ArchiveFile& indexFile() const;

    const eld::archive::ArchiveFile& dataFile(
        std::string_view groupName
    ) const;

    const eld::archive::ArchiveFile& dataFile(
        std::uint16_t fileId
    ) const;

    const eld::image::ImageData& cachedData(
        Key key,
        const eld::archive::ArchiveFile& file
    ) const;

    const SpriteResource& cachedResource(
        Key key,
        const eld::image::ImageData& data
    ) const;

    eld::archive::Archive archive_;
    eld::image::ImageDecoder decoder_;
    SpriteAssembler assembler_;

    mutable std::map<
        Key,
        eld::image::ImageData
    > dataCache_;

    mutable std::map<
        Key,
        SpriteResource
    > resourceCache_;
};

}
