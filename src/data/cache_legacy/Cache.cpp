#include "Cache.h"

#include <algorithm>
#include <string>
#include <utility>

#include "binary/ByteReader.h"

namespace eld::cache_legacy {

namespace {

constexpr std::uint32_t IndexEntrySize = 6;
constexpr std::uint32_t SectorSize = 520;
constexpr std::uint32_t SectorHeaderSize = 8;
constexpr std::uint32_t SectorPayloadSize =
    SectorSize - SectorHeaderSize;

}

Cache::Cache() = default;

Cache::Cache(
    std::filesystem::path rootPath
)
    : rootPath_(std::move(rootPath)) {
}

bool Cache::isValid() const {
    return validateDirectory();
}

bool Cache::hasFile(
    CacheIndex index,
    int fileId
) const {
    return readIndexEntry(
        index,
        fileId
    ).has_value();
}

std::optional<CacheFile> Cache::readFile(
    CacheIndex index,
    int fileId
) const {
    std::optional<CacheIndexEntry> entry =
        readIndexEntry(
            index,
            fileId
        );

    if (!entry.has_value()) {
        return std::nullopt;
    }

    std::ifstream datFile(
        datPath(),
        std::ios::binary
    );

    if (!datFile.is_open()) {
        return std::nullopt;
    }

    std::vector<std::uint8_t> payload;

    payload.reserve(
        entry->size
    );

    std::uint32_t sector =
        entry->firstSector;

    std::uint32_t remainingBytes =
        entry->size;

    std::uint16_t expectedChunk =
        0;

    while (remainingBytes > 0) {
        if (
            !seekToSector(
                datFile,
                sector
            )
        ) {
            return std::nullopt;
        }

        std::optional<CacheSectorHeader> header =
            readSectorHeader(
                datFile
            );

        if (!header.has_value()) {
            return std::nullopt;
        }

        if (
            !validateSectorHeader(
                *header,
                index,
                fileId,
                expectedChunk
            )
        ) {
            return std::nullopt;
        }

        std::optional<std::vector<std::uint8_t>> sectorPayload =
            readSectorPayload(
                datFile,
                remainingBytes
            );

        if (!sectorPayload.has_value()) {
            return std::nullopt;
        }

        payload.insert(
            payload.end(),
            sectorPayload->begin(),
            sectorPayload->end()
        );

        remainingBytes -=
            static_cast<std::uint32_t>(
                sectorPayload->size()
            );

        sector =
            header->nextSector;

        expectedChunk++;
    }

    return CacheFile{
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

    std::ifstream idxFile(
        idxPath(index),
        std::ios::binary
    );

    if (!idxFile.is_open()) {
        return files;
    }

    idxFile.seekg(
        0,
        std::ios::end
    );

    const std::streamsize fileSize =
        idxFile.tellg();

    if (fileSize < 0) {
        return files;
    }

    idxFile.seekg(
        0,
        std::ios::beg
    );

    const int fileCount =
        static_cast<int>(
            fileSize / IndexEntrySize
        );

    for (
        int fileId = 0;
        fileId < fileCount;
        fileId++
    ) {
        std::optional<CacheFile> file =
            readFile(
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

std::filesystem::path Cache::datPath() const {
    return rootPath_ / "main_file_cache.dat";
}

std::filesystem::path Cache::idxPath(
    CacheIndex index
) const {
    return rootPath_ /
        (
            "main_file_cache.idx" +
            std::to_string(
                static_cast<int>(index) - 1
            )
        );
}

bool Cache::validateDirectory() const {
    if (!std::filesystem::exists(rootPath_)) {
        return false;
    }

    if (!std::filesystem::exists(datPath())) {
        return false;
    }

    for (
        int i = static_cast<int>(
            CacheIndex::Config
        );
        i <= static_cast<int>(
            CacheIndex::Map
        );
        i++
    ) {
        if (
            !std::filesystem::exists(
                idxPath(
                    static_cast<CacheIndex>(i)
                )
            )
        ) {
            return false;
        }
    }

    return true;
}

std::optional<CacheIndexEntry>
Cache::readIndexEntry(
    CacheIndex index,
    int fileId
) const {
    if (fileId < 0) {
        return std::nullopt;
    }

    std::ifstream idxFile(
        idxPath(index),
        std::ios::binary
    );

    if (!idxFile.is_open()) {
        return std::nullopt;
    }

    idxFile.seekg(
        static_cast<std::streamoff>(
            fileId
        ) * IndexEntrySize,
        std::ios::beg
    );

    std::optional<std::vector<std::uint8_t>> bytes =
        readBytes(
            idxFile,
            IndexEntrySize
        );

    if (!bytes.has_value()) {
        return std::nullopt;
    }

    try {
        eld::binary::ByteReader reader(
            *bytes
        );

        const CacheIndexEntry entry{
            reader.readU24(),
            reader.readU24()
        };

        if (
            entry.size == 0 ||
            entry.firstSector == 0
        ) {
            return std::nullopt;
        }

        return entry;
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<CacheSectorHeader>
Cache::readSectorHeader(
    std::ifstream& datFile
) const {
    std::optional<std::vector<std::uint8_t>> bytes =
        readBytes(
            datFile,
            SectorHeaderSize
        );

    if (!bytes.has_value()) {
        return std::nullopt;
    }

    try {
        eld::binary::ByteReader reader(
            *bytes
        );

        return CacheSectorHeader{
            reader.readU16(),
            reader.readU16(),
            reader.readU24(),
            static_cast<CacheIndex>(
                reader.readU8()
            )
        };
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<std::vector<std::uint8_t>>
Cache::readSectorPayload(
    std::ifstream& datFile,
    std::uint32_t remainingBytes
) const {
    const std::uint32_t amount =
        std::min(
            remainingBytes,
            SectorPayloadSize
        );

    return readBytes(
        datFile,
        amount
    );
}

bool Cache::seekToSector(
    std::ifstream& datFile,
    std::uint32_t sector
) const {
    if (sector == 0) {
        return false;
    }

    datFile.clear();

    datFile.seekg(
        static_cast<std::streamoff>(
            sector
        ) * SectorSize,
        std::ios::beg
    );

    return datFile.good();
}

bool Cache::validateSectorHeader(
    const CacheSectorHeader& header,
    CacheIndex index,
    int fileId,
    std::uint16_t expectedChunk
) const {
    return
        header.fileId == fileId &&
        header.chunkId == expectedChunk &&
        header.index == index;
}

std::optional<std::vector<std::uint8_t>>
Cache::readBytes(
    std::ifstream& file,
    std::uint32_t amount
) {
    std::vector<std::uint8_t> bytes(
        amount
    );

    file.read(
        reinterpret_cast<char*>(
            bytes.data()
        ),
        static_cast<std::streamsize>(
            amount
        )
    );

    if (
        file.gcount() !=
        static_cast<std::streamsize>(
            amount
        )
    ) {
        return std::nullopt;
    }

    return bytes;
}

}
