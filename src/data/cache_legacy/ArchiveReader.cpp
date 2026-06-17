#include "ArchiveReader.h"

#include <exception>
#include <utility>

#include "binary/ByteReader.h"
#include "binary/Compression.h"

namespace eld::cache_legacy {

namespace {

constexpr std::size_t ArchiveHeaderSize = 6;
constexpr std::size_t FileCountSize = 2;
constexpr std::size_t ArchiveFileEntrySize = 10;

struct FileMetadata {
    int fileIndex = -1;

    std::uint32_t hash = 0;

    std::uint32_t compressedSize = 0;
    std::uint32_t uncompressedSize = 0;

    std::uint32_t offset = 0;
};

std::optional<std::vector<std::uint8_t>>
decodeArchivePayload(
    const std::vector<std::uint8_t>& cacheFilePayload
) {
    if (
        cacheFilePayload.size() <
        ArchiveHeaderSize
    ) {
        return std::nullopt;
    }

    try {
        eld::binary::ByteReader reader(
            cacheFilePayload
        );

        const std::uint32_t uncompressedSize =
            reader.readU24();

        const std::uint32_t compressedSize =
            reader.readU24();

        if (
            compressedSize >
            reader.remaining()
        ) {
            return std::nullopt;
        }

        std::vector<std::uint8_t> payload =
            reader.readBytes(
                compressedSize
            );

        if (
            uncompressedSize ==
            compressedSize
        ) {
            return payload;
        }

        std::vector<std::uint8_t> decompressed =
            eld::binary::decompress(
                payload,
                eld::binary::CompressionType::Bzip2
            );

        if (
            decompressed.size() !=
            uncompressedSize
        ) {
            return std::nullopt;
        }

        return decompressed;
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<std::vector<FileMetadata>>
readFileMetadata(
    const std::vector<std::uint8_t>& archivePayload
) {
    try {
        eld::binary::ByteReader reader(
            archivePayload
        );

        const std::uint16_t fileCount =
            reader.readU16();

        const std::size_t metadataSize =
            FileCountSize +
            static_cast<std::size_t>(
                fileCount
            ) *
            ArchiveFileEntrySize;

        if (
            metadataSize >
            archivePayload.size()
        ) {
            return std::nullopt;
        }

        std::vector<FileMetadata> metadata;

        metadata.reserve(
            fileCount
        );

        std::uint32_t offset =
            static_cast<std::uint32_t>(
                metadataSize
            );

        for (
            std::uint16_t i = 0;
            i < fileCount;
            i++
        ) {
            FileMetadata file{};

            file.fileIndex =
                static_cast<int>(i);

            file.hash =
                reader.readU32();

            file.uncompressedSize =
                reader.readU24();

            file.compressedSize =
                reader.readU24();

            file.offset =
                offset;

            if (
                file.compressedSize >
                archivePayload.size() -
                    static_cast<std::size_t>(
                        offset
                    )
            ) {
                return std::nullopt;
            }

            offset +=
                file.compressedSize;

            metadata.push_back(
                file
            );
        }

        return metadata;
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<std::vector<std::uint8_t>>
extractPayload(
    const std::vector<std::uint8_t>& archivePayload,
    const FileMetadata& file
) {
    const std::size_t offset =
        file.offset;

    const std::size_t compressedSize =
        file.compressedSize;

    if (
        offset >
        archivePayload.size()
    ) {
        return std::nullopt;
    }

    if (
        compressedSize >
        archivePayload.size() - offset
    ) {
        return std::nullopt;
    }

    std::vector<std::uint8_t> payload(
        archivePayload.begin() +
            static_cast<std::ptrdiff_t>(
                offset
            ),
        archivePayload.begin() +
            static_cast<std::ptrdiff_t>(
                offset + compressedSize
            )
    );

    if (
        file.compressedSize ==
        file.uncompressedSize
    ) {
        return payload;
    }

    try {
        std::vector<std::uint8_t> decompressed =
            eld::binary::decompress(
                payload,
                eld::binary::CompressionType::Bzip2
            );

        if (
            decompressed.size() !=
            file.uncompressedSize
        ) {
            return std::nullopt;
        }

        return decompressed;
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

}

std::optional<Archive> ArchiveReader::read(
    const std::vector<std::uint8_t>& cacheFilePayload
) {
    std::optional<std::vector<std::uint8_t>>
        archivePayload =
            decodeArchivePayload(
                cacheFilePayload
            );

    if (!archivePayload.has_value()) {
        return std::nullopt;
    }

    if (
        archivePayload->size() <
        FileCountSize
    ) {
        return std::nullopt;
    }

    std::optional<std::vector<FileMetadata>>
        metadata =
            readFileMetadata(
                *archivePayload
            );

    if (!metadata.has_value()) {
        return std::nullopt;
    }

    Archive archive{};

    archive.files.reserve(
        metadata->size()
    );

    for (
        const FileMetadata& file :
        *metadata
    ) {
        std::optional<std::vector<std::uint8_t>>
            payload =
                extractPayload(
                    *archivePayload,
                    file
                );

        if (!payload.has_value()) {
            return std::nullopt;
        }

        ArchiveFile archiveFile{};

        archiveFile.fileIndex =
            file.fileIndex;

        archiveFile.hash =
            file.hash;

        archiveFile.payload =
            std::move(*payload);

        archive.files.push_back(
            std::move(archiveFile)
        );
    }

    return archive;
}

}
