#include "Writer.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>

#include "binary/ByteReader.h"
#include "binary/ByteWriter.h"
#include "binary/Compression.h"

namespace eld::cache {

namespace {

constexpr std::uint32_t MaximumU24 =
    0x00FFFFFFU;

bool isEmptyEntry(
    const IndexEntry& entry
) {
    return
        entry.size == 0 &&
        entry.firstSector == 0;
}

bool isValidEntry(
    const IndexEntry& entry
) {
    return
        entry.size != 0 &&
        entry.firstSector != 0;
}

void validateEntry(
    const IndexEntry& entry
) {
    if (
        !isEmptyEntry(entry) &&
        !isValidEntry(entry)
    ) {
        throw std::runtime_error(
            "Cache index entry is malformed"
        );
    }
}

std::size_t calculateSectorCount(
    std::size_t byteCount
) {
    return
        (
            byteCount +
            Sector::DataSize -
            1
        ) /
        Sector::DataSize;
}

}

std::uint16_t Writer::createFile(
    const std::filesystem::path& dataPath,
    const Index& index,
    FileData data
) const {
    std::vector<std::uint8_t> bytes =
        prepareBytes(
            std::move(data)
        );

    const std::uint16_t fileId =
        findAvailableFileId(
            index
        );

    std::fstream dataStream(
        dataPath,
        std::ios::binary |
        std::ios::in |
        std::ios::out
    );

    if (!dataStream.is_open()) {
        throw std::runtime_error(
            "Failed to open cache data file: " +
            dataPath.string()
        );
    }

    const std::size_t sectorCount =
        calculateSectorCount(
            bytes.size()
        );

    const std::vector<std::uint32_t> sectorIds =
        allocateSectors(
            dataStream,
            sectorCount
        );

    writePayload(
        dataStream,
        index.id,
        fileId,
        sectorIds,
        bytes
    );

    dataStream.flush();

    if (!dataStream) {
        throw std::runtime_error(
            "Failed to flush cache data file"
        );
    }

    writeEntry(
        index,
        fileId,
        IndexEntry{
            .size =
                static_cast<std::uint32_t>(
                    bytes.size()
                ),
            .firstSector =
                sectorIds.front()
        }
    );

    return fileId;
}

void Writer::updateFile(
    const std::filesystem::path& dataPath,
    const Index& index,
    std::uint16_t fileId,
    FileData data
) const {
    const IndexEntry currentEntry =
        readEntry(
            index,
            fileId
        );

    validateEntry(currentEntry);

    if (isEmptyEntry(currentEntry)) {
        throw std::out_of_range(
            "Cannot update a cache file that does not exist"
        );
    }

    std::vector<std::uint8_t> bytes =
        prepareBytes(
            std::move(data)
        );

    std::fstream dataStream(
        dataPath,
        std::ios::binary |
        std::ios::in |
        std::ios::out
    );

    if (!dataStream.is_open()) {
        throw std::runtime_error(
            "Failed to open cache data file: " +
            dataPath.string()
        );
    }

    const std::size_t sectorCount =
        calculateSectorCount(
            bytes.size()
        );

    const std::vector<std::uint32_t> sectorIds =
        allocateSectors(
            dataStream,
            sectorCount
        );

    /*
     * Write the complete replacement chain before changing
     * the IDX entry. Until writeEntry() succeeds, the old
     * cache file remains valid.
     */
    writePayload(
        dataStream,
        index.id,
        fileId,
        sectorIds,
        bytes
    );

    dataStream.flush();

    if (!dataStream) {
        throw std::runtime_error(
            "Failed to flush cache data file"
        );
    }

    writeEntry(
        index,
        fileId,
        IndexEntry{
            .size =
                static_cast<std::uint32_t>(
                    bytes.size()
                ),
            .firstSector =
                sectorIds.front()
        }
    );
}

std::vector<std::uint8_t> Writer::prepareBytes(
    FileData data
) const {
    std::vector<std::uint8_t> bytes =
        eld::binary::compress(
            data.bytes,
            data.compressionType
        );

    if (bytes.empty()) {
        throw std::invalid_argument(
            "Cache files cannot contain an empty stored payload"
        );
    }

    if (
        bytes.size() >
        MaximumU24
    ) {
        throw std::out_of_range(
            "Cache file size exceeds the 24-bit IDX limit"
        );
    }

    return bytes;
}

std::uint16_t Writer::findAvailableFileId(
    const Index& index
) const {
    std::ifstream stream(
        index.path,
        std::ios::binary |
        std::ios::ate
    );

    if (!stream.is_open()) {
        throw std::runtime_error(
            "Failed to open cache index: " +
            index.path.string()
        );
    }

    const std::streamoff size =
        stream.tellg();

    if (size < 0) {
        throw std::runtime_error(
            "Failed to determine cache index size"
        );
    }

    if (
        size %
        static_cast<std::streamoff>(
            IndexEntry::TotalSize
        ) != 0
    ) {
        throw std::runtime_error(
            "Cache index has an invalid size: " +
            index.path.string()
        );
    }

    const std::uint64_t entryCount =
        static_cast<std::uint64_t>(
            size
        ) /
        IndexEntry::TotalSize;

    if (
        entryCount >
        static_cast<std::uint64_t>(
            std::numeric_limits<std::uint16_t>::max()
        ) + 1
    ) {
        throw std::runtime_error(
            "Cache index contains unsupported file IDs"
        );
    }

    stream.seekg(
        0,
        std::ios::beg
    );

    for (
        std::uint64_t fileId = 0;
        fileId < entryCount;
        ++fileId
    ) {
        std::array<
            std::uint8_t,
            IndexEntry::TotalSize
        > entryBytes{};

        stream.read(
            reinterpret_cast<char*>(
                entryBytes.data()
            ),
            static_cast<std::streamsize>(
                entryBytes.size()
            )
        );

        if (
            stream.gcount() !=
            static_cast<std::streamsize>(
                entryBytes.size()
            )
        ) {
            throw std::runtime_error(
                "Failed to read cache index entry"
            );
        }

        eld::binary::ByteReader reader(
            std::span<const std::uint8_t>(
                entryBytes.data(),
                entryBytes.size()
            )
        );

        const IndexEntry entry{
            .size = reader.readU24(),
            .firstSector = reader.readU24()
        };

        validateEntry(entry);

        if (isEmptyEntry(entry)) {
            return static_cast<std::uint16_t>(
                fileId
            );
        }
    }

    if (
        entryCount >
        std::numeric_limits<std::uint16_t>::max()
    ) {
        throw std::runtime_error(
            "Cache index has no available file IDs"
        );
    }

    return static_cast<std::uint16_t>(
        entryCount
    );
}

IndexEntry Writer::readEntry(
    const Index& index,
    std::uint16_t fileId
) const {
    std::ifstream stream(
        index.path,
        std::ios::binary |
        std::ios::ate
    );

    if (!stream.is_open()) {
        throw std::runtime_error(
            "Failed to open cache index: " +
            index.path.string()
        );
    }

    const std::streamoff size =
        stream.tellg();

    if (size < 0) {
        throw std::runtime_error(
            "Failed to determine cache index size"
        );
    }

    if (
        size %
        static_cast<std::streamoff>(
            IndexEntry::TotalSize
        ) != 0
    ) {
        throw std::runtime_error(
            "Cache index has an invalid size: " +
            index.path.string()
        );
    }

    const std::size_t entryOffset =
        static_cast<std::size_t>(
            fileId
        ) *
        IndexEntry::TotalSize;

    if (
        entryOffset >=
        static_cast<std::size_t>(
            size
        )
    ) {
        return IndexEntry{
            .size = 0,
            .firstSector = 0
        };
    }

    stream.seekg(
        static_cast<std::streamoff>(
            entryOffset
        ),
        std::ios::beg
    );

    if (!stream) {
        throw std::runtime_error(
            "Failed to seek to cache index entry"
        );
    }

    std::array<
        std::uint8_t,
        IndexEntry::TotalSize
    > entryBytes{};

    stream.read(
        reinterpret_cast<char*>(
            entryBytes.data()
        ),
        static_cast<std::streamsize>(
            entryBytes.size()
        )
    );

    if (
        stream.gcount() !=
        static_cast<std::streamsize>(
            entryBytes.size()
        )
    ) {
        throw std::runtime_error(
            "Failed to read cache index entry"
        );
    }

    eld::binary::ByteReader reader(
        std::span<const std::uint8_t>(
            entryBytes.data(),
            entryBytes.size()
        )
    );

    return IndexEntry{
        .size = reader.readU24(),
        .firstSector = reader.readU24()
    };
}

void Writer::writeEntry(
    const Index& index,
    std::uint16_t fileId,
    const IndexEntry& entry
) const {
    if (
        entry.size >
        MaximumU24
    ) {
        throw std::out_of_range(
            "Cache index size exceeds 24 bits"
        );
    }

    if (
        entry.firstSector >
        MaximumU24
    ) {
        throw std::out_of_range(
            "Cache first-sector ID exceeds 24 bits"
        );
    }

    eld::binary::ByteWriter writer(
        IndexEntry::TotalSize
    );

    writer.writeU24(
        entry.size
    );

    writer.writeU24(
        entry.firstSector
    );

    const std::vector<std::uint8_t> bytes =
        writer.takeData();

    std::fstream stream(
        index.path,
        std::ios::binary |
        std::ios::in |
        std::ios::out
    );

    if (!stream.is_open()) {
        throw std::runtime_error(
            "Failed to open cache index: " +
            index.path.string()
        );
    }

    const std::size_t entryOffset =
        static_cast<std::size_t>(
            fileId
        ) *
        IndexEntry::TotalSize;

    stream.seekp(
        static_cast<std::streamoff>(
            entryOffset
        ),
        std::ios::beg
    );

    if (!stream) {
        throw std::runtime_error(
            "Failed to seek to cache index entry"
        );
    }

    stream.write(
        reinterpret_cast<const char*>(
            bytes.data()
        ),
        static_cast<std::streamsize>(
            bytes.size()
        )
    );

    stream.flush();

    if (!stream) {
        throw std::runtime_error(
            "Failed to write cache index entry"
        );
    }
}

std::vector<std::uint32_t>
Writer::allocateSectors(
    std::fstream& dataStream,
    std::size_t sectorCount
) const {
    if (sectorCount == 0) {
        throw std::invalid_argument(
            "Cannot allocate an empty sector chain"
        );
    }

    dataStream.clear();

    dataStream.seekg(
        0,
        std::ios::end
    );

    const std::streamoff size =
        dataStream.tellg();

    if (size < 0) {
        throw std::runtime_error(
            "Failed to determine cache data size"
        );
    }

    if (
        size %
        static_cast<std::streamoff>(
            Sector::TotalSize
        ) != 0
    ) {
        throw std::runtime_error(
            "Cache data file has an invalid size"
        );
    }

    const std::uint64_t existingSectorCount =
        static_cast<std::uint64_t>(
            size
        ) /
        Sector::TotalSize;

    const std::uint64_t firstSectorId =
        std::max<std::uint64_t>(
            1,
            existingSectorCount
        );

    const std::uint64_t lastSectorId =
        firstSectorId +
        sectorCount -
        1;

    if (lastSectorId > MaximumU24) {
        throw std::runtime_error(
            "Cache data file has no remaining 24-bit sector IDs"
        );
    }

    std::vector<std::uint32_t> sectorIds;

    sectorIds.reserve(
        sectorCount
    );

    for (
        std::size_t offset = 0;
        offset < sectorCount;
        ++offset
    ) {
        sectorIds.push_back(
            static_cast<std::uint32_t>(
                firstSectorId +
                offset
            )
        );
    }

    return sectorIds;
}

void Writer::writePayload(
    std::fstream& dataStream,
    IndexId indexId,
    std::uint16_t fileId,
    const std::vector<std::uint32_t>& sectorIds,
    const std::vector<std::uint8_t>& bytes
) const {
    const std::size_t requiredSectorCount =
        calculateSectorCount(
            bytes.size()
        );

    if (
        sectorIds.size() !=
        requiredSectorCount
    ) {
        throw std::invalid_argument(
            "Sector allocation does not match payload size"
        );
    }

    if (
        sectorIds.size() >
        static_cast<std::size_t>(
            std::numeric_limits<std::uint16_t>::max()
        ) + 1
    ) {
        throw std::out_of_range(
            "Cache payload contains too many chunks"
        );
    }

    std::size_t byteOffset = 0;

    for (
        std::size_t chunkId = 0;
        chunkId < sectorIds.size();
        ++chunkId
    ) {
        const std::size_t remaining =
            bytes.size() -
            byteOffset;

        const std::size_t amount =
            std::min(
                remaining,
                Sector::DataSize
            );

        Sector sector{
            .header = SectorHeader{
                .fileId = fileId,
                .chunkId =
                    static_cast<std::uint16_t>(
                        chunkId
                    ),
                .nextSector =
                    chunkId + 1 <
                        sectorIds.size()
                    ? sectorIds[chunkId + 1]
                    : 0,
                .indexId = indexId
            },
            .data = {}
        };

        std::copy_n(
            bytes.begin() +
                static_cast<std::ptrdiff_t>(
                    byteOffset
                ),
            amount,
            sector.data.begin()
        );

        writeSector(
            dataStream,
            sectorIds[chunkId],
            sector
        );

        byteOffset += amount;
    }

    if (byteOffset != bytes.size()) {
        throw std::runtime_error(
            "Failed to write the complete cache payload"
        );
    }
}

void Writer::writeSector(
    std::fstream& dataStream,
    std::uint32_t sectorId,
    const Sector& sector
) const {
    if (
        sectorId == 0 ||
        sectorId > MaximumU24
    ) {
        throw std::out_of_range(
            "Cache sector ID is invalid"
        );
    }

    const std::uint8_t storedIndexId =
        static_cast<std::uint8_t>(
            sector.header.indexId
        ) + 1;

    if (
        storedIndexId == 0 ||
        storedIndexId > 5
    ) {
        throw std::out_of_range(
            "Cache sector index ID is invalid"
        );
    }

    eld::binary::ByteWriter writer(
        Sector::TotalSize
    );

    writer.writeU16(
        sector.header.fileId
    );

    writer.writeU16(
        sector.header.chunkId
    );

    writer.writeU24(
        sector.header.nextSector
    );

    writer.writeU8(
        storedIndexId
    );

    writer.writeBytes(
        std::span<const std::uint8_t>(
            sector.data.data(),
            sector.data.size()
        )
    );

    const std::vector<std::uint8_t> bytes =
        writer.takeData();

    if (
        bytes.size() !=
        Sector::TotalSize
    ) {
        throw std::runtime_error(
            "Serialized cache sector has an invalid size"
        );
    }

    const std::size_t sectorOffset =
        static_cast<std::size_t>(
            sectorId
        ) *
        Sector::TotalSize;

    dataStream.clear();

    dataStream.seekp(
        static_cast<std::streamoff>(
            sectorOffset
        ),
        std::ios::beg
    );

    if (!dataStream) {
        throw std::runtime_error(
            "Failed to seek to cache sector"
        );
    }

    dataStream.write(
        reinterpret_cast<const char*>(
            bytes.data()
        ),
        static_cast<std::streamsize>(
            bytes.size()
        )
    );

    if (!dataStream) {
        throw std::runtime_error(
            "Failed to write cache sector"
        );
    }
}

}
