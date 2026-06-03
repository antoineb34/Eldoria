#include <algorithm>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include <vector>

#include "../../core/cache/Cache.h"
#include "../../core/cache/ArchiveHashes.h"
#include "../../core/compression/Compression.h"
#include "../../core/io/ByteBuffer.h"

namespace {

struct ArchiveEntry {
    uint32_t hash = 0;
    uint32_t compressedSize = 0;
    uint32_t offset = 0;
};

uint16_t readU16At(const std::vector<uint8_t>& data, size_t offset) {
    return
        (static_cast<uint16_t>(data[offset]) << 8) |
        static_cast<uint16_t>(data[offset + 1]);
}

uint32_t readU32At(const std::vector<uint8_t>& data, size_t offset) {
    return
        (static_cast<uint32_t>(data[offset]) << 24) |
        (static_cast<uint32_t>(data[offset + 1]) << 16) |
        (static_cast<uint32_t>(data[offset + 2]) << 8) |
        static_cast<uint32_t>(data[offset + 3]);
}

std::vector<ArchiveEntry> readArchiveEntries(
    const std::vector<uint8_t>& archiveData
) {
    rf::io::ByteBuffer buffer(archiveData);

    uint16_t fileCount = buffer.readU16();

    std::vector<ArchiveEntry> entries;
    entries.reserve(fileCount);

    uint32_t dataOffset = 2 + fileCount * 10;

    for (int i = 0; i < fileCount; i++) {
        ArchiveEntry entry {};

        entry.hash = buffer.readU32();

        uint32_t uncompressedSize = buffer.readU24();
        entry.compressedSize = buffer.readU24();
        entry.offset = dataOffset;

        dataOffset += entry.compressedSize;

        entries.push_back(entry);

        (void)uncompressedSize;
    }

    return entries;
}

std::vector<uint8_t> extractEntry(
    const std::vector<uint8_t>& archiveData,
    const ArchiveEntry& entry
) {
    return {
        archiveData.begin() + entry.offset,
        archiveData.begin() + entry.offset + entry.compressedSize
    };
}

std::vector<uint8_t> readVersionListArchive5(
    rf::cache::Cache& cache
) {
    auto file =
        cache.readFile(
            rf::cache::CacheIndex::Config,
            5
        );

    if (!file.has_value()) {
        throw std::runtime_error("Could not read idx0 archive 5.");
    }

    rf::io::ByteBuffer header(file->payload);

    uint32_t uncompressedSize = header.readU24();
    uint32_t compressedSize = header.readU24();

    std::vector<uint8_t> payload(
        file->payload.begin() + 6,
        file->payload.end()
    );

    if (uncompressedSize != compressedSize) {
        return rf::compression::decompressBzip2(
            payload,
            uncompressedSize
        );
    }

    return payload;
}

struct ModelTables {
    std::vector<uint8_t> index;
    std::vector<uint8_t> crc;
    std::vector<uint8_t> version;
};

ModelTables extractModelTables(
    const std::vector<uint8_t>& archiveData
) {
    using namespace rf::cache::archive;

    ModelTables tables {};

    auto entries = readArchiveEntries(archiveData);

    for (const ArchiveEntry& entry : entries) {
        auto data = extractEntry(archiveData, entry);

        if (entry.hash == ModelIndex) {
            tables.index = data;
        }
        else if (entry.hash == ModelCrc) {
            tables.crc = data;
        }
        else if (entry.hash == ModelVersion) {
            tables.version = data;
        }
    }

    return tables;
}

void printModelMetadata(
    const ModelTables& tables,
    int modelId
) {
    uint8_t flag = tables.index[modelId];
    uint32_t crc = readU32At(tables.crc, modelId * 4);
    uint16_t version = readU16At(tables.version, modelId * 2);

    std::cout
        << "model=" << modelId
        << " flag=0x"
        << std::hex
        << std::setw(2)
        << std::setfill('0')
        << static_cast<int>(flag)
        << std::dec
        << std::setfill(' ')
        << " version=" << version
        << " crc=" << crc
        << "\n";
}

void compareModelFiles(
    rf::cache::Cache& cache,
    const ModelTables& tables,
    int a,
    int b
) {
    std::cout
        << "\n==============================\n"
        << "COMPARE MODEL " << a << " vs " << b << "\n"
        << "==============================\n";

    printModelMetadata(tables, a);
    printModelMetadata(tables, b);

    auto modelA =
        cache.readFile(
            rf::cache::CacheIndex::Model,
            a
        );

    auto modelB =
        cache.readFile(
            rf::cache::CacheIndex::Model,
            b
        );

    if (!modelA.has_value()) {
        std::cout << "Could not read model " << a << "\n";
        return;
    }

    if (!modelB.has_value()) {
        std::cout << "Could not read model " << b << "\n";
        return;
    }

    std::cout
        << "model " << a << " payload size="
        << modelA->payload.size()
        << "\n";

    std::cout
        << "model " << b << " payload size="
        << modelB->payload.size()
        << "\n";

    bool sameBytes =
        modelA->payload == modelB->payload;

    std::cout
        << "same raw payload bytes="
        << (sameBytes ? "YES" : "NO")
        << "\n";

    if (!sameBytes) {
        size_t minSize =
            std::min(
                modelA->payload.size(),
                modelB->payload.size()
            );

        for (size_t i = 0; i < minSize; i++) {
            if (modelA->payload[i] != modelB->payload[i]) {
                std::cout
                    << "first different byte offset="
                    << i
                    << " a=0x"
                    << std::hex
                    << static_cast<int>(modelA->payload[i])
                    << " b=0x"
                    << static_cast<int>(modelB->payload[i])
                    << std::dec
                    << "\n";
                break;
            }
        }
    }
}

}

int main() {
    try {
        rf::cache::Cache cache;

        if (!cache.isValid()) {
            std::cerr << "Cache directory is invalid.\n";
            return 1;
        }

        auto archiveData =
            readVersionListArchive5(cache);

        ModelTables modelTables =
            extractModelTables(archiveData);

        std::cout
            << "model_index entries="
            << modelTables.index.size()
            << "\n";

        std::cout
            << "model_crc entries="
            << modelTables.crc.size() / 4
            << "\n";

        std::cout
            << "model_version entries="
            << modelTables.version.size() / 2
            << "\n";

        compareModelFiles(cache, modelTables, 28, 86);
        compareModelFiles(cache, modelTables, 31, 89);
        compareModelFiles(cache, modelTables, 34, 92);
        compareModelFiles(cache, modelTables, 36, 94);

        return 0;
    }
    catch (const std::exception& error) {
        std::cerr
            << "Error: "
            << error.what()
            << "\n";

        return 1;
    }
}
