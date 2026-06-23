#include "ArchiveParser.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <utility>

#include "binary/ByteReader.h"
#include "binary/Compression.h"

namespace eld::archive {

std::optional<Archive> ArchiveParser::parse(
    const std::vector<std::uint8_t>& cacheFilePayload
) const {
    if (!validatePayload(cacheFilePayload)) {
        return std::nullopt;
    }

    try {
        const ArchiveHeader header =
            readHeader(
                cacheFilePayload
            );

        std::vector<std::uint8_t> payload =
            decodePayload(
                cacheFilePayload,
                header
            );

        if (payload.size() < 2) {
            return std::nullopt;
        }

        const std::uint16_t fileCount =
            readFileCount(
                payload
            );

        const ArchiveSections sections =
            calculateSections(
                fileCount,
                payload.size()
            );

        if (
            !validateSections(
                sections,
                payload.size()
            )
        ) {
            return std::nullopt;
        }

        std::vector<ArchiveFile> files =
            readFiles(
                payload,
                fileCount,
                sections
            );

        return Archive(
            std::move(payload),
            header,
            sections,
            std::move(files)
        );
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

bool ArchiveParser::validatePayload(
    const std::vector<std::uint8_t>& cacheFilePayload
) const {
    constexpr std::size_t HeaderSize = 6;

    return
        cacheFilePayload.size() >=
        HeaderSize;
}

ArchiveHeader ArchiveParser::readHeader(
    const std::vector<std::uint8_t>& cacheFilePayload
) const {
    eld::binary::ByteReader reader(
        cacheFilePayload
    );

    return ArchiveHeader{
        .uncompressedSize = reader.readU24(),
        .compressedSize = reader.readU24()
    };
}

std::vector<std::uint8_t>
ArchiveParser::decodePayload(
    const std::vector<std::uint8_t>& cacheFilePayload,
    const ArchiveHeader& header
) const {
    constexpr std::size_t HeaderSize = 6;

    if (
        header.compressedSize !=
        cacheFilePayload.size() - HeaderSize
    ) {
        throw std::runtime_error(
            "Archive payload size does not match its header"
        );
    }

    eld::binary::ByteReader reader(
        cacheFilePayload
    );

    reader.setPosition(
        HeaderSize
    );

    std::vector<std::uint8_t> payload =
        reader.readBytes(
            header.compressedSize
        );

    if (!header.compressed()) {
        if (
            payload.size() !=
            header.uncompressedSize
        ) {
            throw std::runtime_error(
                "Archive payload has an invalid size"
            );
        }

        return payload;
    }

    std::vector<std::uint8_t> decompressed =
        eld::binary::decompress(
            payload,
            eld::binary::CompressionType::Bzip2
        );

    if (
        decompressed.size() !=
        header.uncompressedSize
    ) {
        throw std::runtime_error(
            "Archive decompressed to an unexpected size"
        );
    }

    return decompressed;
}

std::uint16_t ArchiveParser::readFileCount(
    const std::vector<std::uint8_t>& payload
) const {
    eld::binary::ByteReader reader(
        payload
    );

    return reader.readU16();
}

ArchiveSections ArchiveParser::calculateSections(
    std::uint16_t fileCount,
    std::size_t payloadSize
) const {
    constexpr std::size_t FileCountSize = 2;
    constexpr std::size_t FileTableEntrySize = 10;

    ArchiveSections sections{};

    sections.fileCount = ArchiveSection{
        .offset = 0,
        .size = FileCountSize
    };

    sections.fileTable = ArchiveSection{
        .offset = sections.fileCount.end(),
        .size =
            static_cast<std::size_t>(
                fileCount
            ) *
            FileTableEntrySize
    };

    sections.fileData = ArchiveSection{
        .offset = sections.fileTable.end(),
        .size =
            sections.fileTable.end() <= payloadSize
                ? payloadSize - sections.fileTable.end()
                : 0
    };

    return sections;
}

bool ArchiveParser::validateSections(
    const ArchiveSections& sections,
    std::size_t payloadSize
) const {
    return
        sections.fileCount.offset == 0 &&
        sections.fileCount.size == 2 &&
        sections.fileCount.end() <= payloadSize &&
        sections.fileTable.offset ==
            sections.fileCount.end() &&
        sections.fileTable.end() <= payloadSize &&
        sections.fileData.offset ==
            sections.fileTable.end() &&
        sections.fileData.end() == payloadSize;
}

std::vector<ArchiveFile> ArchiveParser::readFiles(
    const std::vector<std::uint8_t>& payload,
    std::uint16_t fileCount,
    const ArchiveSections& sections
) const {
    constexpr std::size_t FileTableEntrySize = 10;

    eld::binary::ByteReader reader(
        payload
    );

    reader.setPosition(
        sections.fileTable.offset
    );

    std::vector<ArchiveFile> files;

    files.reserve(
        fileCount
    );

    std::size_t dataOffset =
        sections.fileData.offset;

    for (
        std::uint16_t id = 0;
        id < fileCount;
        id++
    ) {
        const std::size_t tableOffset =
            reader.position();

        ArchiveFile file{};

        file.id = id;
        file.nameHash = reader.readU32();
        file.uncompressedSize = reader.readU24();
        file.compressedSize = reader.readU24();

        file.tableEntry = ArchiveSection{
            .offset = tableOffset,
            .size = FileTableEntrySize
        };

        if (
            dataOffset > payload.size() ||
            file.compressedSize >
                payload.size() - dataOffset
        ) {
            throw std::runtime_error(
                "Archive file data lies outside the payload"
            );
        }

        file.data = ArchiveSection{
            .offset = dataOffset,
            .size = file.compressedSize
        };

        file.payload =
            readFilePayload(
                payload,
                file
            );

        dataOffset =
            file.data.end();

        files.push_back(
            std::move(file)
        );
    }

    if (dataOffset != payload.size()) {
        throw std::runtime_error(
            "Archive contains unclaimed file data"
        );
    }

    return files;
}

std::vector<std::uint8_t>
ArchiveParser::readFilePayload(
    const std::vector<std::uint8_t>& payload,
    const ArchiveFile& file
) const {
    const auto begin =
        payload.begin() +
        static_cast<std::ptrdiff_t>(
            file.data.offset
        );

    const auto end =
        payload.begin() +
        static_cast<std::ptrdiff_t>(
            file.data.end()
        );

    std::vector<std::uint8_t> filePayload(
        begin,
        end
    );

    if (!file.compressed()) {
        if (
            filePayload.size() !=
            file.uncompressedSize
        ) {
            throw std::runtime_error(
                "Archive file has an invalid size"
            );
        }

        return filePayload;
    }

    std::vector<std::uint8_t> decompressed =
        eld::binary::decompress(
            filePayload,
            eld::binary::CompressionType::Bzip2
        );

    if (
        decompressed.size() !=
        file.uncompressedSize
    ) {
        throw std::runtime_error(
            "Archive file decompressed to an unexpected size"
        );
    }

    return decompressed;
}

}
