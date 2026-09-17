#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "Archive.h"

namespace eld::archive {

class ArchiveParser {
public:
    std::optional<Archive> parse(
        const std::vector<std::uint8_t>& cacheFilePayload
    ) const;

private:
    bool validatePayload(
        const std::vector<std::uint8_t>& cacheFilePayload
    ) const;

    ArchiveHeader readHeader(
        const std::vector<std::uint8_t>& cacheFilePayload
    ) const;

    std::vector<std::uint8_t> decodePayload(
        const std::vector<std::uint8_t>& cacheFilePayload,
        const ArchiveHeader& header
    ) const;

    std::uint16_t readFileCount(
        const std::vector<std::uint8_t>& payload
    ) const;

    ArchiveSections calculateSections(
        std::uint16_t fileCount,
        std::size_t payloadSize
    ) const;

    bool validateSections(
        const ArchiveSections& sections,
        std::size_t payloadSize
    ) const;

    std::vector<ArchiveFile> readFiles(
        const std::vector<std::uint8_t>& payload,
        std::uint16_t fileCount,
        const ArchiveSections& sections
    ) const;

    std::vector<std::uint8_t> readFilePayload(
        const std::vector<std::uint8_t>& payload,
        const ArchiveFile& file
    ) const;
};

}
