#include "TextureLoader.h"

#include <bzlib.h>

#include "../../io/ByteBuffer.h"

namespace rf::texture {

namespace {

constexpr int TextureArchiveId = 6;

constexpr int ArchiveHeaderSize = 6;
constexpr int ArchiveFileEntrySize = 10;

}

TextureLoader::TextureLoader(
    const rf::cache::Cache& cache
)
    : cache_(cache)
{
}

std::optional<TextureAsset> TextureLoader::load(
    std::uint32_t id
) {
    auto cached =
        textureCache_.find(id);

    if (cached != textureCache_.end()) {
        return cached->second;
    }

    auto file =
        getTextureFile(id);

    if (!file.has_value()) {
        return std::nullopt;
    }

    TextureAsset asset =
        textureBuilder_.build(
            *file
        );

    textureCache_[id] =
        asset;

    return asset;
}

std::optional<TextureFile>
TextureLoader::getTextureFile(
    std::uint32_t id
) const {
    auto archive =
        getTextureArchive();

    if (!archive.has_value()) {
        return std::nullopt;
    }

    auto entries =
        readArchiveFileEntries(
            *archive
        );

    if (!entries.has_value()) {
        return std::nullopt;
    }

    if (
        entries->empty() ||
        id >= entries->size() - 1
    ) {
        return std::nullopt;
    }

    const ArchiveFileEntry& textureEntry =
        (*entries)[id];

    const ArchiveFileEntry& indexEntry =
        entries->back();

    auto indexData =
        readArchiveFile(
            *archive,
            indexEntry
        );

    auto textureData =
        readArchiveFile(
            *archive,
            textureEntry
        );

    if (
        !indexData.has_value() ||
        !textureData.has_value()
    ) {
        return std::nullopt;
    }

    return fileReader_.read(
        static_cast<int>(id),
        *indexData,
        *textureData
    );
}

std::optional<std::vector<uint8_t>>
TextureLoader::getTextureArchive() const {
    auto archive =
        cache_.readFile(
            rf::cache::CacheIndex::Config,
            TextureArchiveId
        );

    if (!archive.has_value()) {
        return std::nullopt;
    }

    return archive->payload;
}

std::optional<std::vector<TextureLoader::ArchiveFileEntry>>
TextureLoader::readArchiveFileEntries(
    const std::vector<uint8_t>& archive
) const {
    if (
        archive.size() <
        ArchiveHeaderSize + 2
    ) {
        return std::nullopt;
    }

    rf::io::ByteBuffer buffer(
        archive
    );

    uint32_t uncompressedSize =
        buffer.readU24();

    uint32_t compressedSize =
        buffer.readU24();

    if (
        uncompressedSize !=
        compressedSize
    ) {
        return std::nullopt;
    }

    uint16_t fileCount =
        buffer.readU16();

    std::vector<ArchiveFileEntry> entries;

    entries.reserve(
        fileCount
    );

    uint32_t offset =
        ArchiveHeaderSize +
        2 +
        fileCount * ArchiveFileEntrySize;

    for (
        int i = 0;
        i < fileCount;
        i++
    ) {
        ArchiveFileEntry entry {};

        entry.hash =
            buffer.readU32();

        entry.uncompressedSize =
            buffer.readU24();

        entry.compressedSize =
            buffer.readU24();

        entry.offset =
            offset;

        offset +=
            entry.compressedSize;

        entries.push_back(
            entry
        );
    }

    return entries;
}

std::optional<std::vector<uint8_t>>
TextureLoader::readArchiveFile(
    const std::vector<uint8_t>& archive,
    const ArchiveFileEntry& entry
) const {
    if (
        entry.offset + entry.compressedSize >
        archive.size()
    ) {
        return std::nullopt;
    }

    std::vector<uint8_t> payload(
        archive.begin() + entry.offset,
        archive.begin() + entry.offset + entry.compressedSize
    );

    if (
        entry.compressedSize ==
        entry.uncompressedSize
    ) {
        return payload;
    }

    return decompressBzip(
        payload,
        entry.uncompressedSize
    );
}

std::optional<std::vector<uint8_t>>
TextureLoader::decompressBzip(
    const std::vector<uint8_t>& payload,
    uint32_t expectedSize
) const {
    std::vector<char> compressed;

    compressed.reserve(
        payload.size() + 4
    );

    compressed.push_back('B');
    compressed.push_back('Z');
    compressed.push_back('h');
    compressed.push_back('1');

    for (uint8_t byte : payload) {
        compressed.push_back(
            static_cast<char>(byte)
        );
    }

    std::vector<uint8_t> output(
        expectedSize
    );

    unsigned int outputSize =
        expectedSize;

    int result =
        BZ2_bzBuffToBuffDecompress(
            reinterpret_cast<char*>(output.data()),
            &outputSize,
            compressed.data(),
            static_cast<unsigned int>(compressed.size()),
            0,
            0
        );

    if (result != BZ_OK) {
        return std::nullopt;
    }

    output.resize(
        outputSize
    );

    return output;
}

}
