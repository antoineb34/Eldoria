#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "../compression/Compression.h"
#include "Cache.h"

namespace rf::cache {

struct ArchiveFileDetails {
    int index = 0;
    std::uint32_t hash = 0;
    std::uint32_t uncompressedSize = 0;
    std::uint32_t compressedSize = 0;
    std::size_t payloadSize = 0;
};

struct CacheFileDetails {
    CacheIndex index = CacheIndex::Config;
    int fileId = -1;
    std::size_t payloadSize = 0;
    int cacheEntrySize = 0;
    int firstSector = 0;
    rf::compression::CompressionType compressionType =
        rf::compression::CompressionType::Unknown;

    bool isArchive = false;
    std::vector<ArchiveFileDetails> archiveFiles;
};

CacheFileDetails inspectCacheFile(
    const CacheFile& file
);

}
