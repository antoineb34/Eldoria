#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "TextureAsset.h"
#include "TextureBuilder.h"
#include "TextureFile.h"
#include "TextureFileReader.h"

#include "../../cache/Cache.h"

namespace rf::texture {

class TextureLoader {
public:
    explicit TextureLoader(
        const rf::cache::Cache& cache
    );

    std::optional<TextureAsset> load(
        std::uint32_t id
    );

private:
    struct ArchiveFileEntry {
        uint32_t hash = 0;

        uint32_t uncompressedSize = 0;
        uint32_t compressedSize = 0;

        uint32_t offset = 0;
    };

private:
    std::optional<TextureFile> getTextureFile(
        std::uint32_t id
    ) const;

    std::optional<std::vector<uint8_t>>
    getTextureArchive() const;

    std::optional<std::vector<ArchiveFileEntry>>
    readArchiveFileEntries(
        const std::vector<uint8_t>& archive
    ) const;

    std::optional<std::vector<uint8_t>>
    readArchiveFile(
        const std::vector<uint8_t>& archive,
        const ArchiveFileEntry& entry
    ) const;

    std::optional<std::vector<uint8_t>>
    decompressBzip(
        const std::vector<uint8_t>& payload,
        uint32_t expectedSize
    ) const;

private:
    const rf::cache::Cache& cache_;

    TextureFileReader fileReader_;
    TextureBuilder textureBuilder_;

    std::unordered_map<
        std::uint32_t,
        TextureAsset
    > textureCache_;
};

}
