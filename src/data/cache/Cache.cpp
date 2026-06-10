#include "Cache.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>

namespace rf::cache {

namespace {

constexpr int IndexEntrySize = 6;
constexpr int SectorSize = 520;
constexpr int SectorHeaderSize = 8;
constexpr int SectorPayloadSize = SectorSize - SectorHeaderSize;

std::filesystem::path defaultCachePath() {
    const std::array<std::filesystem::path, 4> candidates {
        "cache",
        "data/cache",
        "../cache",
        "../../cache"
    };

    for (const auto& candidate : candidates) {
        if (
            std::filesystem::exists(candidate / "main_file_cache.dat") &&
            std::filesystem::exists(candidate / "main_file_cache.idx0")
        ) {
            return candidate;
        }
    }

    return "cache";
}

int readU24(
    const unsigned char* bytes
) {
    return
        (static_cast<int>(bytes[0]) << 16) |
        (static_cast<int>(bytes[1]) << 8) |
        static_cast<int>(bytes[2]);
}

int readU16(
    const unsigned char* bytes
) {
    return
        (static_cast<int>(bytes[0]) << 8) |
        static_cast<int>(bytes[1]);
}

} // namespace

Cache::Cache()
    : rootPath_(defaultCachePath())
{
}

Cache::Cache(
    std::filesystem::path rootPath
)
    : rootPath_(std::move(rootPath))
{
}

bool Cache::isValid() const {
    if (!std::filesystem::exists(rootPath_ / "main_file_cache.dat")) {
        return false;
    }

    for (int index = 0; index <= 4; ++index) {
        if (!std::filesystem::exists(rootPath_ / ("main_file_cache.idx" + std::to_string(index)))) {
            return false;
        }
    }

    return true;
}

std::filesystem::path Cache::indexPath(
    CacheIndex index
) const {
    return rootPath_ / (
        "main_file_cache.idx" +
        std::to_string(static_cast<int>(index))
    );
}

std::optional<CacheIndexEntry> Cache::readIndexEntry(
    CacheIndex index,
    int fileId
) const {
    if (fileId < 0) {
        return std::nullopt;
    }

    std::ifstream stream(
        indexPath(index),
        std::ios::binary
    );

    if (!stream) {
        return std::nullopt;
    }

    stream.seekg(
        static_cast<std::streamoff>(fileId) * IndexEntrySize,
        std::ios::beg
    );

    unsigned char bytes[IndexEntrySize] {};
    stream.read(
        reinterpret_cast<char*>(bytes),
        IndexEntrySize
    );

    if (stream.gcount() != IndexEntrySize) {
        return std::nullopt;
    }

    CacheIndexEntry entry;
    entry.size = readU24(bytes);
    entry.firstSector = readU24(bytes + 3);

    if (entry.size <= 0 || entry.firstSector <= 0) {
        return std::nullopt;
    }

    return entry;
}

std::optional<CacheFile> Cache::readFile(
    CacheIndex index,
    int fileId
) const {
    auto entry = readIndexEntry(
        index,
        fileId
    );

    if (!entry.has_value()) {
        return std::nullopt;
    }

    std::ifstream dataStream(
        rootPath_ / "main_file_cache.dat",
        std::ios::binary
    );

    if (!dataStream) {
        return std::nullopt;
    }

    std::vector<unsigned char> payload;
    payload.reserve(
        static_cast<std::size_t>(entry->size)
    );

    int currentSector = entry->firstSector;
    int expectedChunk = 0;

    while (
        currentSector > 0 &&
        static_cast<int>(payload.size()) < entry->size
    ) {
        dataStream.seekg(
            static_cast<std::streamoff>(currentSector) * SectorSize,
            std::ios::beg
        );

        unsigned char header[SectorHeaderSize] {};
        dataStream.read(
            reinterpret_cast<char*>(header),
            SectorHeaderSize
        );

        if (dataStream.gcount() != SectorHeaderSize) {
            return std::nullopt;
        }

        int actualFileId = readU16(header);
                int actualChunk = readU16(header + 2);
                int nextSector = readU24(header + 4);
                int actualIndex = static_cast<int>(header[7]);

                // Cache format uses 1-based indexing for the index field in sector headers
                // (1=Config, 2=Models, 3=Animations, 4=Midi, 5=Maps)
                // while CacheIndex enum is 0-based.
                int expectedCacheIndex = static_cast<int>(index) + 1;

                if (
                    actualFileId != fileId ||
                    actualChunk != expectedChunk ||
                    actualIndex != expectedCacheIndex
                ) {
                    return std::nullopt;
                }

                int remaining =
                    entry->size - static_cast<int>(payload.size());

                int bytesToRead = std::min(
                    SectorPayloadSize,
                    remaining
                );

                std::array<unsigned char, SectorPayloadSize> sectorPayload {};
                dataStream.read(
            reinterpret_cast<char*>(sectorPayload.data()),
            bytesToRead
        );

        if (dataStream.gcount() != bytesToRead) {
            return std::nullopt;
        }

        payload.insert(
            payload.end(),
            sectorPayload.begin(),
            sectorPayload.begin() + bytesToRead
        );

        currentSector = nextSector;
        ++expectedChunk;
    }

    if (static_cast<int>(payload.size()) != entry->size) {
        return std::nullopt;
    }

    return CacheFile {
        fileId,
        index,
        *entry,
        std::move(payload)
    };
}

std::vector<CacheFile> Cache::listFiles(
    CacheIndex index
) const {
    std::vector<CacheFile> files;

    std::ifstream stream(
        indexPath(index),
        std::ios::binary | std::ios::ate
    );

    if (!stream) {
        return files;
    }

    int fileCount =
        static_cast<int>(stream.tellg()) / IndexEntrySize;

    for (int fileId = 0; fileId < fileCount; ++fileId) {
        auto file = readFile(
            index,
            fileId
        );

        if (file.has_value()) {
            files.push_back(
                std::move(*file)
            );
        }
    }

    return files;
}

} // namespace rf::cache
