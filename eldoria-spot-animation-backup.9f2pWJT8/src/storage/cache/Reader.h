#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>

#include "File.h"
#include "Index.h"
#include "Payload.h"
#include "Sector.h"

namespace eld::cache {

class Reader {
public:
    File readFile(
        const std::filesystem::path& dataPath,
        const Index& index,
        std::uint16_t fileId
    ) const;

    std::optional<IndexEntry> findEntry(
        const Index& index,
        std::uint16_t fileId
    ) const;

private:
    IndexEntry readEntry(
        const Index& index,
        std::uint16_t fileId
    ) const;

    Payload readPayload(
        const std::filesystem::path& dataPath,
        const Index& index,
        std::uint16_t fileId,
        const IndexEntry& entry
    ) const;

    Sector readSector(
        std::ifstream& dataStream,
        std::uint32_t sectorId,
        std::size_t requiredDataBytes
    ) const;

    eld::binary::CompressionType
    readCompressionType(
        const Payload& payload,
        std::uint32_t fileSize
    ) const;
};

}
