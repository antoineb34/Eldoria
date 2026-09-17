#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "image/ImageData.h"
#include "image/ImageDecoder.h"

namespace eld::texture
{

    class TextureLoader
    {
    public:
        explicit TextureLoader(
            const eld::cache::Cache &cache);

        const eld::image::ImageData &get(
            std::uint16_t id) const;

        const eld::image::ImageData *find(
            std::uint16_t id) const;

        std::vector<std::uint16_t> listIds() const;

        bool contains(
            std::uint16_t id) const;

        std::size_t count() const;

    private:
        static constexpr auto CacheIndex =
            eld::cache::IndexId::Config;

        static constexpr std::uint16_t
            TextureArchiveId = 6;

        static constexpr std::string_view
            IndexFileName = "index.dat";

        static constexpr std::string_view
            TextureFileExtension = ".dat";

        eld::image::ImageData load(
            std::uint16_t id) const;

        std::string fileName(
            std::uint16_t id) const;

        eld::archive::Archive archive_;
        eld::image::ImageDecoder decoder_;

        mutable std::unordered_map<
            std::uint16_t,
            eld::image::ImageData>
            textureCache_;
    };

}
