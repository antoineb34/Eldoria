#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "Texture.h"
#include "archive/Archive.h"
#include "cache/Store.h"
#include "image/ImageDecoder.h"

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

    eld::archive::Archive archive_;
    eld::image::ImageDecoder decoder_;
};

}
