#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace eld::archive {

struct ArchiveSection {
    std::size_t offset = 0;
    std::size_t size = 0;

    std::size_t end() const {
        return offset + size;
    }

    bool present() const {
        return size > 0;
    }
};

struct ArchiveFile {
    std::uint16_t id = 0;
    std::uint32_t nameHash = 0;

    std::uint32_t uncompressedSize = 0;
    std::uint32_t compressedSize = 0;

    ArchiveSection tableEntry;
    ArchiveSection data;

    std::vector<std::uint8_t> payload;

    bool compressed() const {
        return compressedSize != uncompressedSize;
    }
};

}
