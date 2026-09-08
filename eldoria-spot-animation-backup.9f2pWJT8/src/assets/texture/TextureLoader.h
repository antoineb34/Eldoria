#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "image/ImageData.h"
#include "image/ImageDecoder.h"
#include "texture/TextureAssembler.h"
#include "texture/TextureResource.h"

namespace eld::texture {

class TextureLoader {
public:
    explicit TextureLoader(
        const eld::cache::Cache& cache
    );

    const eld::image::ImageData& data(
        std::uint16_t id
    ) const;

    const TextureResource& resource(
        std::uint16_t id
    ) const;

    std::optional<TextureResource> find(
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

    eld::image::ImageData loadData(
        std::uint16_t id
    ) const;

    eld::archive::Archive archive_;
    eld::image::ImageDecoder decoder_;
    TextureAssembler assembler_;

    mutable std::map<
        std::uint16_t,
        eld::image::ImageData
    > dataCache_;

    mutable std::map<
        std::uint16_t,
        TextureResource
    > resourceCache_;
};

}
