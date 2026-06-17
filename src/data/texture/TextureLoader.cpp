#include "TextureLoader.h"

#include "cache_legacy/ArchiveReader.h"

namespace eld::texture {

namespace {

constexpr int TextureArchiveId = 6;

}

TextureLoader::TextureLoader(
    const eld::cache_legacy::Cache& cache
)
    : cache_(cache)
{
}

std::optional<TextureAsset> TextureLoader::load(
    std::uint32_t id
) {
    auto cached =
        textureCache_.find(id);

    if (cached != textureCache_.end()) {
        return cached->second;
    }

    auto file =
        getTextureFile(id);

    if (!file.has_value()) {
        return std::nullopt;
    }

    TextureAsset asset =
        textureBuilder_.build(
            *file
        );

    textureCache_[id] =
        asset;

    return asset;
}

std::optional<TextureFile>
TextureLoader::getTextureFile(
    std::uint32_t id
) const {
    auto cacheFile =
        cache_.readFile(
            eld::cache_legacy::CacheIndex::Config,
            TextureArchiveId
        );

    if (!cacheFile.has_value()) {
        return std::nullopt;
    }

    auto archive =
        eld::cache_legacy::ArchiveReader::read(
            cacheFile->payload
        );

    if (!archive.has_value()) {
        return std::nullopt;
    }

    if (
        archive->files.empty() ||
        id >= archive->files.size() - 1
    ) {
        return std::nullopt;
    }

    auto textureFile =
        archive->findByIndex(
            static_cast<int>(id)
        );

    auto indexFile =
        archive->findByIndex(
            static_cast<int>(
                archive->files.size() - 1
            )
        );

    if (
        !textureFile.has_value() ||
        !indexFile.has_value()
    ) {
        return std::nullopt;
    }

    return fileReader_.read(
        static_cast<int>(id),
        indexFile->payload,
        textureFile->payload
    );
}

}
