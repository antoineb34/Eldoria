#include "VersionListReader.h"

#include "../ArchiveHashes.h"
#include "../ArchiveReader.h"
#include "../Cache.h"
#include "../../binary/ByteBuffer.h"

namespace rf::cache::versionlist {

namespace {

constexpr int VersionListArchiveId = 5;

}

std::optional<ModelEntry> VersionList::getModel(
    int id
) const {
    if (
        id < 0 ||
        id >= static_cast<int>(models.size())
    ) {
        return std::nullopt;
    }

    return models[id];
}

std::optional<VersionList> readVersionList(
    const rf::cache::Cache& cache
) {
    auto cacheFile =
        cache.readFile(
            rf::cache::CacheIndex::Config,
            VersionListArchiveId
        );

    if (!cacheFile.has_value()) {
        return std::nullopt;
    }

    auto archive =
        rf::cache::ArchiveReader::read(
            cacheFile->payload
        );

    if (!archive.has_value()) {
        return std::nullopt;
    }

    auto modelIndexFile =
        archive->findByHash(
            rf::cache::archive::ModelIndex
        );

    auto modelCrcFile =
        archive->findByHash(
            rf::cache::archive::ModelCrc
        );

    auto modelVersionFile =
        archive->findByHash(
            rf::cache::archive::ModelVersion
        );

    if (
        !modelIndexFile.has_value() ||
        !modelCrcFile.has_value() ||
        !modelVersionFile.has_value()
    ) {
        return std::nullopt;
    }

    const std::vector<unsigned char>& modelIndexData =
        modelIndexFile->payload;

    const std::vector<unsigned char>& modelCrcData =
        modelCrcFile->payload;

    const std::vector<unsigned char>& modelVersionData =
        modelVersionFile->payload;

    const std::size_t modelCount =
        modelIndexData.size();

    if (
        modelCrcData.size() < modelCount * 4 ||
        modelVersionData.size() < modelCount * 2
    ) {
        return std::nullopt;
    }

    rf::io::ByteBuffer crcBuffer(
        modelCrcData
    );

    rf::io::ByteBuffer versionBuffer(
        modelVersionData
    );

    VersionList versionList {};
    versionList.models.reserve(
        modelCount
    );

    for (std::size_t id = 0; id < modelCount; id++) {
        ModelEntry entry {};

        entry.id =
            static_cast<int>(id);

        entry.flags =
            modelIndexData[id];

        entry.crc =
            crcBuffer.readU32();

        entry.version =
            versionBuffer.readU16();

        versionList.models.push_back(
            entry
        );
    }

    return versionList;
}

}
