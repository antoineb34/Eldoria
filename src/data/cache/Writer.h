#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

#include "File.h"
#include "Index.h"
#include "Sector.h"

namespace eld::cache {

class Writer {
public:
    std::uint16_t createFile(
        const std::filesystem::path& dataPath,
        const Index& index,
        FileData data
    ) const;

    void updateFile(
        const std::filesystem::path& dataPath,
        const Index& index,
        std::uint16_t fileId,
        FileData data
    ) const;

private:
    std::vector<std::uint8_t> prepareBytes(
        FileData data
    ) const;

    std::uint16_t findAvailableFileId(
        const Index& index
    ) const;

    IndexEntry readEntry(
        const Index& index,
        std::uint16_t fileId
    ) const;

    void writeEntry(
        const Index& index,
        std::uint16_t fileId,
        const IndexEntry& entry
    ) const;

    std::vector<std::uint32_t> allocateSectors(
        std::fstream& dataStream,
        std::size_t sectorCount
    ) const;

    void writePayload(
        std::fstream& dataStream,
        IndexId indexId,
        std::uint16_t fileId,
        const std::vector<std::uint32_t>& sectorIds,
        const std::vector<std::uint8_t>& bytes
    ) const;

    void writeSector(
        std::fstream& dataStream,
        std::uint32_t sectorId,
        const Sector& sector
    ) const;
};

}
