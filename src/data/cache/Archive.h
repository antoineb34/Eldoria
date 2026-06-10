#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace rf::cache {

struct ArchiveFile {
    int fileIndex = -1;

    std::uint32_t hash = 0;

    std::uint32_t uncompressedSize = 0;
    std::uint32_t compressedSize = 0;

    std::vector<unsigned char> payload;
};

struct Archive {
    std::vector<ArchiveFile> files;

    std::optional<ArchiveFile> findByHash(
        std::uint32_t hash
    ) const;

    std::optional<ArchiveFile> findByIndex(
        int fileIndex
    ) const;
};

}
