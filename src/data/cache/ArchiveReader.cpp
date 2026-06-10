#include "ArchiveReader.h"

#include "../binary/ByteBuffer.h"
#include "../compression/Compression.h"

namespace rf::cache {

namespace {

constexpr int ArchiveHeaderSize = 6;
constexpr int FileCountSize = 2;
constexpr int ArchiveFileEntrySize = 10;

struct FileMetadata {
    int fileIndex = -1;

    std::uint32_t hash = 0;

    std::uint32_t compressedSize = 0;
    std::uint32_t uncompressedSize = 0;

    std::uint32_t offset = 0;
};

std::optional<std::vector<unsigned char>> decodeArchivePayload(
    const std::vector<unsigned char>& cacheFilePayload
) {
    if (cacheFilePayload.size() < ArchiveHeaderSize) {
        return std::nullopt;
    }

    rf::io::ByteBuffer buffer(
        cacheFilePayload
    );

    std::uint32_t uncompressedSize =
        buffer.readU24();

    std::uint32_t compressedSize =
        buffer.readU24();

    if (cacheFilePayload.size() < ArchiveHeaderSize + compressedSize) {
        return std::nullopt;
    }

    std::vector<unsigned char> payload(
        cacheFilePayload.begin() + ArchiveHeaderSize,
        cacheFilePayload.begin() + ArchiveHeaderSize + compressedSize
    );

    if (uncompressedSize == compressedSize) {
        return payload;
    }

    auto decompressed = rf::compression::decompressBzip2(
        payload,
        uncompressedSize
    );

    if (decompressed.empty() && uncompressedSize > 0) {
        return std::nullopt;
    }

    return decompressed;
}

std::vector<FileMetadata> readFileMetadata(
    const std::vector<unsigned char>& archivePayload
) {
    rf::io::ByteBuffer buffer(
        archivePayload
    );

    std::uint16_t fileCount =
        buffer.readU16();

    std::vector<FileMetadata> metadata;

    metadata.reserve(
        fileCount
    );

    std::uint32_t offset =
        FileCountSize +
        fileCount * ArchiveFileEntrySize;

    for (int i = 0; i < fileCount; i++) {
        FileMetadata file {};

        file.fileIndex =
            i;

        file.hash =
            buffer.readU32();

        file.uncompressedSize =
            buffer.readU24();

        file.compressedSize =
            buffer.readU24();

        file.offset =
            offset;

        offset +=
            file.compressedSize;

        metadata.push_back(
            file
        );
    }

    return metadata;
}

std::optional<std::vector<unsigned char>> extractPayload(
    const std::vector<unsigned char>& archivePayload,
    const FileMetadata& file
) {
    if (
        file.offset > archivePayload.size() ||
        file.offset + file.compressedSize > archivePayload.size()
    ) {
        return std::nullopt;
    }

    std::vector<unsigned char> payload(
        archivePayload.begin() + file.offset,
        archivePayload.begin() + file.offset + file.compressedSize
    );

    if (file.compressedSize == file.uncompressedSize) {
        return payload;
    }

    auto decompressed = rf::compression::decompressBzip2(
        payload,
        file.uncompressedSize
    );

    if (decompressed.empty() && file.uncompressedSize > 0) {
        return std::nullopt;
    }

    return decompressed;
}

}

std::optional<Archive> ArchiveReader::read(
    const std::vector<unsigned char>& cacheFilePayload
) {
    auto archivePayload =
        decodeArchivePayload(
            cacheFilePayload
        );

    if (!archivePayload.has_value()) {
        return std::nullopt;
    }

    if (archivePayload->size() < FileCountSize) {
        return std::nullopt;
    }

    std::vector<FileMetadata> metadata =
        readFileMetadata(
            *archivePayload
        );

    Archive archive {};

    archive.files.reserve(
        metadata.size()
    );

    for (const FileMetadata& file : metadata) {
        auto payload =
            extractPayload(
                *archivePayload,
                file
            );

        if (!payload.has_value()) {
            return std::nullopt;
        }

        ArchiveFile archiveFile {};

        archiveFile.fileIndex =
            file.fileIndex;

        archiveFile.hash =
            file.hash;

        archiveFile.uncompressedSize =
            file.uncompressedSize;

        archiveFile.compressedSize =
            file.compressedSize;

        archiveFile.payload =
            std::move(*payload);

        archive.files.push_back(
            std::move(archiveFile)
        );
    }

    return archive;
}

}
