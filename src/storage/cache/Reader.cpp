#include "Reader.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#include "binary/ByteReader.h"
#include "binary/Compression.h"

namespace eld::cache {

namespace {

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

}

File Reader::readFile(
    const std::filesystem::path& dataPath,
    const Index& index,
    std::uint16_t fileId
) const {
    const IndexEntry entry =
        readEntry(
            index,
            fileId
        );

    validateEntry(entry);

    if (isEmptyEntry(entry)) {
        throw std::runtime_error(
            "Cache file does not exist"
        );
    }

    Payload payload =
        readPayload(
            dataPath,
            index,
            fileId,
            entry
        );

    const eld::binary::CompressionType compressionType =
        readCompressionType(
            payload,
            entry.size
        );

    return File(
        fileId,
        entry,
        std::move(payload),
        compressionType
    );
}

std::optional<IndexEntry> Reader::findEntry(
    const Index& index,
    std::uint16_t fileId
) const {
    try {
        const IndexEntry entry =
            readEntry(
                index,
                fileId
            );

        validateEntry(entry);

        if (isEmptyEntry(entry)) {
            return std::nullopt;
        }

        return entry;
    }
    catch (const std::out_of_range&) {
        return std::nullopt;
    }
}

IndexEntry Reader::readEntry(
    const Index& index,
    std::uint16_t fileId
) const {
    std::ifstream indexStream(
        index.path,
        std::ios::binary
    );

    if (!indexStream.is_open()) {
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

    indexStream.seekg(
        static_cast<std::streamoff>(
            entryOffset
        ),
        std::ios::beg
    );

    if (!indexStream) {
        throw std::out_of_range(
            "Cache index entry lies outside the index file"
        );
    }

    std::array<
        std::uint8_t,
        IndexEntry::TotalSize
    > entryBytes{};

    indexStream.read(
        reinterpret_cast<char*>(
            entryBytes.data()
        ),
        static_cast<std::streamsize>(
            entryBytes.size()
        )
    );

    if (
        indexStream.gcount() !=
        static_cast<std::streamsize>(
            entryBytes.size()
        )
    ) {
        throw std::out_of_range(
            "Cache index entry lies outside the index file"
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

Payload Reader::readPayload(
    const std::filesystem::path& dataPath,
    const Index& index,
    std::uint16_t fileId,
    const IndexEntry& entry
) const {
    std::ifstream dataStream(
        dataPath,
        std::ios::binary
    );

    if (!dataStream.is_open()) {
        throw std::runtime_error(
            "Failed to open cache data file: " +
            dataPath.string()
        );
    }

    const std::size_t expectedSectorCount =
        (
            static_cast<std::size_t>(
                entry.size
            ) +
            Sector::DataSize -
            1
        ) /
        Sector::DataSize;

    std::vector<Sector> sectors;

    sectors.reserve(
        expectedSectorCount
    );

    std::uint32_t currentSectorId =
        entry.firstSector;

    for (
        std::size_t expectedChunkId = 0;
        expectedChunkId < expectedSectorCount;
        ++expectedChunkId
    ) {
        if (currentSectorId == 0) {
            throw std::runtime_error(
                "Cache sector chain ended before the complete file was read"
            );
        }

        const std::size_t bytesAlreadyRead =
            expectedChunkId *
            Sector::DataSize;

        const std::size_t remainingBytes =
            static_cast<std::size_t>(
                entry.size
            ) -
            bytesAlreadyRead;

        const std::size_t requiredDataBytes =
            std::min(
                remainingBytes,
                Sector::DataSize
            );

        Sector sector =
            readSector(
                dataStream,
                currentSectorId,
                requiredDataBytes
            );

        if (
            sector.header.fileId !=
            fileId
        ) {
            throw std::runtime_error(
                "Cache sector contains an unexpected file ID"
            );
        }

        if (
            sector.header.chunkId !=
            expectedChunkId
        ) {
            throw std::runtime_error(
                "Cache sector contains an unexpected chunk ID"
            );
        }

        if (
            sector.header.indexId !=
            index.id
        ) {
            throw std::runtime_error(
                "Cache sector belongs to a different index"
            );
        }

        currentSectorId =
            sector.header.nextSector;

        sectors.push_back(
            std::move(sector)
        );
    }

    if (currentSectorId != 0) {
        throw std::runtime_error(
            "Cache sector chain continues beyond the declared file size"
        );
    }

    return Payload(
        std::move(sectors)
    );
}

Sector Reader::readSector(
    std::ifstream& dataStream,
    std::uint32_t sectorId,
    std::size_t requiredDataBytes
) const {
    if (
        requiredDataBytes == 0 ||
        requiredDataBytes > Sector::DataSize
    ) {
        throw std::invalid_argument(
            "Invalid cache sector payload size"
        );
    }

    const std::size_t sectorOffset =
        static_cast<std::size_t>(
            sectorId
        ) *
        Sector::TotalSize;

    dataStream.clear();

    dataStream.seekg(
        static_cast<std::streamoff>(
            sectorOffset
        ),
        std::ios::beg
    );

    if (!dataStream) {
        throw std::out_of_range(
            "Cache sector lies outside the data file"
        );
    }

    std::array<
        std::uint8_t,
        Sector::TotalSize
    > sectorBytes{};

    const std::size_t requiredSectorBytes =
        Sector::HeaderSize +
        requiredDataBytes;

    dataStream.read(
        reinterpret_cast<char*>(
            sectorBytes.data()
        ),
        static_cast<std::streamsize>(
            requiredSectorBytes
        )
    );

    if (
        dataStream.gcount() !=
        static_cast<std::streamsize>(
            requiredSectorBytes
        )
    ) {
        throw std::out_of_range(
            "Cache sector is incomplete"
        );
    }

    eld::binary::ByteReader reader(
        std::span<const std::uint8_t>(
            sectorBytes.data(),
            requiredSectorBytes
        )
    );

    const std::uint16_t fileId =
        reader.readU16();

    const std::uint16_t chunkId =
        reader.readU16();

    const std::uint32_t nextSector =
        reader.readU24();

    const std::uint8_t storedIndexId =
        reader.readU8();

    if (
        storedIndexId == 0 ||
        storedIndexId > 5
    ) {
        throw std::runtime_error(
            "Cache sector contains an invalid index ID"
        );
    }

    Sector sector{
        .header = SectorHeader{
            .fileId = fileId,
            .chunkId = chunkId,
            .nextSector = nextSector,
            .indexId = static_cast<IndexId>(
                storedIndexId - 1
            )
        },
        .data = {}
    };

    const std::span<const std::uint8_t> sectorData =
        reader.readSpan(
            requiredDataBytes
        );

    std::copy(
        sectorData.begin(),
        sectorData.end(),
        sector.data.begin()
    );

    return sector;
}

eld::binary::CompressionType
Reader::readCompressionType(
    const Payload& payload,
    std::uint32_t fileSize
) const {
    std::vector<std::uint8_t> bytes;

    bytes.reserve(
        fileSize
    );

    for (
        const Sector& sector :
        payload.getSectors()
    ) {
        const std::size_t remaining =
            static_cast<std::size_t>(
                fileSize
            ) -
            bytes.size();

        const std::size_t amount =
            std::min(
                remaining,
                sector.data.size()
            );

        bytes.insert(
            bytes.end(),
            sector.data.begin(),
            sector.data.begin() +
                static_cast<std::ptrdiff_t>(
                    amount
                )
        );

        if (
            bytes.size() ==
            fileSize
        ) {
            break;
        }
    }

    if (
        bytes.size() !=
        fileSize
    ) {
        throw std::runtime_error(
            "Cache payload does not contain the complete file"
        );
    }

    return eld::binary::getCompressionType(
        bytes
    );
}

}
