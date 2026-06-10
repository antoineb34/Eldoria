#include "ArchiveReader.h"

#include "binary/ByteBuffer.h"
#include "binary/Compression.h"

namespace eld::cache {

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

std::optional<std::vector<std::uint8_t>> decodeArchivePayload(
    const std::vector<std::uint8_t>& cacheFilePayload
) {
    if (cacheFilePayload.size() < ArchiveHeaderSize) {
        return std::nullopt;
    }

    eld::binary::ByteBuffer buffer(
        cacheFilePayload
    );

    std::uint32_t uncompressedSize =
        buffer.readU24();

    std::uint32_t compressedSize =
        buffer.readU24();

    std::vector<std::uint8_t> payload(
        cacheFilePayload.begin() + ArchiveHeaderSize,
        cacheFilePayload.end()
    );

    if (uncompressedSize == compressedSize) {
        return payload;
    }

    return eld::binary::decompressBzip2(
        payload,
        uncompressedSize
    );
}

std::vector<FileMetadata> readFileMetadata(
    const std::vector<std::uint8_t>& archivePayload
) {
    eld::binary::ByteBuffer buffer(
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

std::optional<std::vector<std::uint8_t>> extractPayload(
    const std::vector<std::uint8_t>& archivePayload,
    const FileMetadata& file
) {
    if (
        file.offset > archivePayload.size() ||
        file.offset + file.compressedSize > archivePayload.size()
    ) {
        return std::nullopt;
    }

    std::vector<std::uint8_t> payload(
        archivePayload.begin() + file.offset,
        archivePayload.begin() + file.offset + file.compressedSize
    );

    if (file.compressedSize == file.uncompressedSize) {
        return payload;
    }

    return eld::binary::decompressBzip2(
        payload,
        file.uncompressedSize
    );
}

}

std::optional<Archive> ArchiveReader::read(
    const std::vector<std::uint8_t>& cacheFilePayload
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

        archiveFile.payload =
            std::move(*payload);

        archive.files.push_back(
            std::move(archiveFile)
        );
    }

    return archive;
}

}
