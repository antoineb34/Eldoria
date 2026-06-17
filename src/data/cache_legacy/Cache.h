#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <vector>

#include "CacheTypes.h"

namespace eld::cache_legacy {

class Cache {
public:
    Cache();

    explicit Cache(
        std::filesystem::path rootPath
    );

    bool isValid() const;

    bool hasFile(
        CacheIndex index,
        int fileId
    ) const;

    std::optional<CacheFile> readFile(
        CacheIndex index,
        int fileId
    ) const;

    std::vector<CacheFile> listFiles(
        CacheIndex index
    ) const;

private:
    std::filesystem::path datPath() const;

    std::filesystem::path idxPath(
        CacheIndex index
    ) const;

    bool validateDirectory() const;

    std::optional<CacheIndexEntry> readIndexEntry(
        CacheIndex index,
        int fileId
    ) const;

    std::optional<CacheSectorHeader> readSectorHeader(
        std::ifstream& datFile
    ) const;

    std::optional<std::vector<std::uint8_t>> readSectorPayload(
        std::ifstream& datFile,
        std::uint32_t remainingBytes
    ) const;

    bool seekToSector(
        std::ifstream& datFile,
        std::uint32_t sector
    ) const;

    bool validateSectorHeader(
        const CacheSectorHeader& header,
        CacheIndex index,
        int fileId,
        std::uint16_t expectedChunk
    ) const;

    static std::optional<std::vector<std::uint8_t>> readBytes(
        std::ifstream& file,
        std::uint32_t amount
    );

private:
    std::filesystem::path rootPath_ = "cache";
};

}
