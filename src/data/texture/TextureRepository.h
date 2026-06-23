#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

#include "Texture.h"
#include "TextureDecoder.h"
#include "TextureFileParser.h"
#include "archive/Archive.h"
#include "cache/Store.h"

namespace eld::texture {

using TexturePredicate =
    std::function<bool(const Texture&)>;

class TextureRepository {
public:
    explicit TextureRepository(
        eld::cache::Store store
    );

    Texture get(
        std::uint16_t id
    ) const;

    std::optional<Texture> find(
        std::uint16_t id
    ) const;

    TextureFile getFile(
        std::uint16_t id
    ) const;

    TextureImage getImage(
        std::uint16_t id
    ) const;

    std::vector<std::uint16_t> listIds() const;

    std::vector<std::uint16_t> filterIds(
        const TexturePredicate& predicate
    ) const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

    std::size_t count(
        const TexturePredicate& predicate
    ) const;

private:
    static eld::archive::Archive loadArchive(
        const eld::cache::Store& store
    );

    const eld::archive::ArchiveFile& getIndexFile() const;

    const eld::archive::ArchiveFile& getDataFile(
        std::uint16_t id
    ) const;

    eld::archive::Archive archive_;

    TextureFileParser parser_;
    TextureDecoder decoder_;
};

}
