#include "ArchiveReader.h"

#include <bzlib.h>

#include <cstdint>

namespace rf::cache {

namespace {

std::uint16_t readU16(
    const std::vector<unsigned char>& data,
    std::size_t offset
) {
    return
        (static_cast<std::uint16_t>(data[offset]) << 8) |
        static_cast<std::uint16_t>(data[offset + 1]);
}

std::uint32_t readU24(
    const std::vector<unsigned char>& data,
    std::size_t offset
) {
    return
        (static_cast<std::uint32_t>(data[offset]) << 16) |
        (static_cast<std::uint32_t>(data[offset + 1]) << 8) |
        static_cast<std::uint32_t>(data[offset + 2]);
}

std::uint32_t readU32(
    const std::vector<unsigned char>& data,
    std::size_t offset
) {
    return
        (static_cast<std::uint32_t>(data[offset]) << 24) |
        (static_cast<std::uint32_t>(data[offset + 1]) << 16) |
        (static_cast<std::uint32_t>(data[offset + 2]) << 8) |
        static_cast<std::uint32_t>(data[offset + 3]);
}

std::vector<unsigned char> decompressBzip2Payload(
    const std::vector<unsigned char>& payload,
    std::size_t expectedSize
) {
    std::vector<unsigned char> input;

    if (
        payload.size() >= 3 &&
        payload[0] == 'B' &&
        payload[1] == 'Z' &&
        payload[2] == 'h'
    ) {
        input = payload;
    }
    else {
        input = { 'B', 'Z', 'h', '1' };
        input.insert(
            input.end(),
            payload.begin(),
            payload.end()
        );
    }

    std::vector<unsigned char> output(expectedSize);

    unsigned int outputSize =
        static_cast<unsigned int>(output.size());

    int result = BZ2_bzBuffToBuffDecompress(
        reinterpret_cast<char*>(output.data()),
        &outputSize,
        const_cast<char*>(
            reinterpret_cast<const char*>(input.data())
        ),
        static_cast<unsigned int>(input.size()),
        0,
        1
    );

    if (result != BZ_OK) {
        return {};
    }

    output.resize(outputSize);
    return output;
}

} // namespace

std::optional<Archive> ArchiveReader::read(
    const std::vector<unsigned char>& payload
) {
    if (payload.size() < 8) {
        return std::nullopt;
    }

    std::uint32_t uncompressedSize = readU24(
        payload,
        0
    );

    std::uint32_t compressedSize = readU24(
        payload,
        3
    );

    if (payload.size() < 6 + compressedSize) {
        return std::nullopt;
    }

    std::vector<unsigned char> archiveData;

    if (uncompressedSize != compressedSize) {
        std::vector<unsigned char> compressed(
            payload.begin() + 6,
            payload.begin() + 6 + compressedSize
        );

        archiveData = decompressBzip2Payload(
            compressed,
            uncompressedSize
        );

        if (archiveData.empty()) {
            return std::nullopt;
        }
    }
    else {
        archiveData.assign(
            payload.begin() + 6,
            payload.begin() + 6 + compressedSize
        );
    }

    if (archiveData.size() < 2) {
        return std::nullopt;
    }

    std::uint16_t fileCount = readU16(
        archiveData,
        0
    );

    std::size_t tableOffset = 2;
    std::size_t dataOffset = tableOffset + fileCount * 10;

    if (archiveData.size() < dataOffset) {
        return std::nullopt;
    }

    Archive archive;
    archive.files.reserve(fileCount);

    for (std::uint16_t fileIndex = 0; fileIndex < fileCount; ++fileIndex) {
        ArchiveFile file;
        file.hash = readU32(
            archiveData,
            tableOffset
        );
        tableOffset += 4;

        file.uncompressedSize = readU24(
            archiveData,
            tableOffset
        );
        tableOffset += 3;

        file.compressedSize = readU24(
            archiveData,
            tableOffset
        );
        tableOffset += 3;

        if (archiveData.size() < dataOffset + file.compressedSize) {
            return std::nullopt;
        }

        file.payload.assign(
            archiveData.begin() + static_cast<std::ptrdiff_t>(dataOffset),
            archiveData.begin() + static_cast<std::ptrdiff_t>(dataOffset + file.compressedSize)
        );

        dataOffset += file.compressedSize;

        archive.files.push_back(
            std::move(file)
        );
    }

    return archive;
}

} // namespace rf::cache
