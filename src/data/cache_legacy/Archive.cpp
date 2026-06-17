#include "Archive.h"

namespace eld::cache_legacy {

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
    int fileIndex
) const {
    for (const ArchiveFile& file : files) {
        if (file.fileIndex == fileIndex) {
            return file;
        }
    }

    return std::nullopt;
}

}
