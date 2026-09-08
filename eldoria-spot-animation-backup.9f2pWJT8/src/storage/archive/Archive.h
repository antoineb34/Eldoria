#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

#include "ArchiveFile.h"

namespace eld::cache {
class Store;

}

namespace eld::archive {

struct ArchiveHeader {
    std::uint32_t uncompressedSize = 0;
    std::uint32_t compressedSize = 0;

    bool compressed() const {
        return compressedSize != uncompressedSize;
    }
};

struct ArchiveSections {
    ArchiveSection fileCount;
    ArchiveSection fileTable;
    ArchiveSection fileData;
};

class Archive {
public:
    Archive(
        std::vector<std::uint8_t> payload,
        ArchiveHeader header,
        ArchiveSections sections,
        std::vector<ArchiveFile> files
    );

    const std::vector<std::uint8_t>& getPayload() const;

    const ArchiveHeader& getHeader() const;

    const ArchiveSections& getSections() const;

    const ArchiveFile& get(
        std::uint16_t id
    ) const;

    const ArchiveFile& get(
        std::string_view name
    ) const;

    const ArchiveFile* find(
        std::uint16_t id
    ) const;

    const ArchiveFile* find(
        std::string_view name
    ) const;

    const std::vector<ArchiveFile>& list() const;

    bool contains(
        std::uint16_t id
    ) const;

    bool contains(
        std::string_view name
    ) const;

    std::size_t count() const;

private:
    std::vector<std::uint8_t> payload_;

    ArchiveHeader header_;
    ArchiveSections sections_;

    std::vector<ArchiveFile> files_;
};

Archive load(
    const eld::cache::Store& store,
    std::uint16_t archiveId
);

}
