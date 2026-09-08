#include "sequence/SequencePipeline.h"

#include <exception>
#include <stdexcept>
#include <string>

#include "binary/ByteReader.h"

namespace eld::sequence {

SequencePipeline::SequencePipeline(
    const eld::cache::Cache& cache
)
    : archive_(
          eld::archive::load(
              cache.open(Index),
              ArchiveId
          )
      ) {
}


SequencePipeline::SequencePipeline(
    const eld::cache::Cache& cache,
    const eld::animation::AnimationFrameTable& frames
)
    : SequencePipeline(cache) {
    assembler_ = SequenceAssembler(frames);
}


const SequenceData& SequencePipeline::data(
    std::uint16_t id
) const {
    const auto existing =
        dataCache_.find(id);

    if (existing != dataCache_.end()) {
        return existing->second;
    }

    const auto inserted =
        dataCache_.emplace(
            id,
            loadData(id)
        );

    return inserted.first->second;
}


SequenceData SequencePipeline::loadData(
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
            "sequence data/index count mismatch"
        );
    }

    if (id >= dataCount) {
        throw std::out_of_range(
            "sequence does not exist"
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
                "sequence asset exceeds data file"
            );
        }

        dataReader.readBytes(size);
    }

    const auto size = indexReader.readU16();

    if (!dataReader.canRead(size)) {
        throw std::runtime_error(
            "sequence asset exceeds data file"
        );
    }

    const std::vector<std::uint8_t> payload =
        dataReader.readBytes(size);

    try {
        SequenceData sequence =
            decoder_.decode(payload);

        sequence.id = id;
        return sequence;
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode sequence " +
            std::to_string(id) +
            ": " +
            error.what()
        );
    }
}


const SequenceResource&
SequencePipeline::resource(
    std::uint16_t id
) const {
    const auto existing =
        resourceCache_.find(id);

    if (existing != resourceCache_.end()) {
        return existing->second;
    }

    const auto inserted =
        resourceCache_.emplace(
            id,
            assembler_.assemble(
                data(id)
            )
        );

    return inserted.first->second;
}


std::optional<SequenceResource>
SequencePipeline::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return resource(id);
}


std::vector<std::uint16_t>
SequencePipeline::listIds() const {
    const std::size_t assetCount = count();

    std::vector<std::uint16_t> ids;
    ids.reserve(assetCount);

    for (
        std::size_t id = 0;
        id < assetCount;
        ++id
    ) {
        ids.push_back(
            static_cast<std::uint16_t>(id)
        );
    }

    return ids;
}


bool SequencePipeline::contains(
    std::uint16_t id
) const {
    return id < count();
}


std::size_t SequencePipeline::count() const {
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
            "sequence data/index count mismatch"
        );
    }

    return dataCount;
}

}
