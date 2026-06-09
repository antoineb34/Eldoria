#include "VersionListReader.h"

#include "../ArchiveReader.h"

namespace rf::cache::versionlist {

std::optional<VersionList> VersionListReader::read(
    const std::vector<unsigned char>& payload
) {
    auto archive = rf::cache::ArchiveReader::read(
        payload
    );

    if (!archive.has_value()) {
        return std::nullopt;
    }

    return VersionList {
        *archive
    };
}

}
