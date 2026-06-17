#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace eld::cache_legacy {

struct ArchiveFile {
    int fileIndex = -1;
    std::uint32_t hash = 0;
    std::vector<std::uint8_t> payload;
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
