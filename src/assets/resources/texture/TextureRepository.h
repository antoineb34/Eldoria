#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "Texture.h"
#include "image/IndexedImageDecoder.h"
#include "archive/Archive.h"
#include "cache/Store.h"

namespace eld::texture {

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

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    static eld::archive::Archive loadArchive(
        const eld::cache::Store& store
    );

    const eld::archive::ArchiveFile& getIndexFile() const;

    const eld::archive::ArchiveFile& getDataFile(
        std::uint16_t id
    ) const;

    eld::archive::Archive archive_;
    eld::image::IndexedImageDecoder decoder_;
};

}
