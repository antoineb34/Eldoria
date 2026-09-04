#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "Texture.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "decoders/ImageDecoder.h"

namespace eld::texture {

class TextureRepository {
public:
    explicit TextureRepository(
        const eld::cache::Cache& cache
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
    static constexpr auto Index =
        eld::cache::IndexId::Config;

    static constexpr std::uint16_t ArchiveId = 6;

    eld::archive::Archive archive_;
    eld::image::ImageDecoder decoder_;
};

}
