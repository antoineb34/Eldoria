#include "spot_animation/SpotAnimationLoader.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

#include "binary/ByteReader.h"

namespace eld::spot_animation {

SpotAnimationLoader::SpotAnimationLoader(
    const eld::cache::Cache& cache
)
    : archive_(eld::archive::load(cache.open(Index), ArchiveId)) {
}

SpotAnimationLoader::SpotAnimationLoader(
    const eld::cache::Cache& cache,
    const eld::model::ModelLoader& models,
    const eld::sequence::SequenceLoader& sequences
)
    : SpotAnimationLoader(cache) {
    assembler_ = SpotAnimationAssembler(models, sequences);
}

const SpotAnimationData& SpotAnimationLoader::data(
    std::uint16_t id
) const {
    const auto existing = dataCache_.find(id);

    if (existing != dataCache_.end()) {
        return existing->second;
    }

    return dataCache_.emplace(
        id,
        loadData(id)
    ).first->second;
}

SpotAnimationData SpotAnimationLoader::loadData(
    std::uint16_t id
) const {
    const eld::archive::ArchiveFile& dataFile =
        archive_.get(DataFile);

    const eld::archive::ArchiveFile& indexFile =
        archive_.get(IndexFile);

    eld::binary::ByteReader dataReader(
        dataFile.payload
    );

    eld::binary::ByteReader indexReader(
        indexFile.payload
    );

    const auto dataCount = dataReader.readU16();
    const auto indexCount = indexReader.readU16();

    if (dataCount != indexCount) {
        throw std::runtime_error(
            "spot animation data/index count mismatch"
        );
    }

    if (id >= dataCount) {
        throw std::out_of_range(
            "spot animation does not exist"
        );
    }

    for (
        std::uint16_t currentId = 0;
        currentId < id;
        ++currentId
    ) {
        const auto size = indexReader.readU16();

        if (!dataReader.canRead(size)) {
            throw std::runtime_error(
                "spot animation asset exceeds data file"
            );
        }

        dataReader.readBytes(size);
    }

    const auto size = indexReader.readU16();

    if (!dataReader.canRead(size)) {
        throw std::runtime_error(
            "spot animation asset exceeds data file"
        );
    }

    const std::vector<std::uint8_t> data =
        dataReader.readBytes(size);

    try {
        SpotAnimationData asset =
            decoder_.decode(data);

        asset.id = id;

        return asset;
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode spot animation " +
            std::to_string(id) +
            ": " +
            error.what()
        );
    }
}

std::optional<SpotAnimationData> SpotAnimationLoader::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return data(id);
}

const SpotAnimationResource& SpotAnimationLoader::resource(
    std::uint16_t id
) const {
    const auto existing = resourceCache_.find(id);

    if (existing != resourceCache_.end()) {
        return existing->second;
    }

    return resourceCache_.emplace(
        id,
        assembler_.assemble(data(id))
    ).first->second;
}

std::vector<std::uint16_t> SpotAnimationLoader::listIds() const {
    std::vector<std::uint16_t> ids;
    const std::size_t assetCount = count();

    ids.reserve(assetCount);

    for (std::size_t id = 0; id < assetCount; ++id) {
        ids.push_back(static_cast<std::uint16_t>(id));
    }
    return ids;
}

bool SpotAnimationLoader::contains(
    std::uint16_t id
) const {
    return id < count();
}

std::size_t SpotAnimationLoader::count() const {
    eld::binary::ByteReader dataReader(archive_.get(DataFile).payload);
    eld::binary::ByteReader indexReader(archive_.get(IndexFile).payload);
    const auto dataCount = dataReader.readU16();
    const auto indexCount = indexReader.readU16();
    if (dataCount != indexCount) {
        throw std::runtime_error("spot animation data/index count mismatch");
    }
    return dataCount;
}

}
