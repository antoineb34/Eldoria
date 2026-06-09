#include "Archive.h"

namespace rf::cache {

std::optional<ArchiveFile> Archive::findByHash(
    std::uint32_t hash
) const {
    for (const ArchiveFile& file : files) {
        if (file.hash == hash) {
            return file;
        }
    }

    return std::nullopt;
}

std::optional<ArchiveFile> Archive::findByIndex(
    int index
) const {
    if (
        index < 0 ||
        index >= static_cast<int>(files.size())
    ) {
        return std::nullopt;
    }

    return files[
        static_cast<std::size_t>(index)
    ];
}

}
