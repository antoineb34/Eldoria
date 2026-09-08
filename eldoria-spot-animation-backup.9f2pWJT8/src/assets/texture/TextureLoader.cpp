#include "texture/TextureLoader.h"

#include <exception>
#include <limits>
#include <stdexcept>
#include <string>

namespace eld::texture {

TextureLoader::TextureLoader(
    const eld::cache::Cache& cache
)
    : archive_(
          eld::archive::load(
              cache.open(Index),
              ArchiveId
          )
      ) {
}


const eld::image::ImageData&
TextureLoader::data(
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


eld::image::ImageData
TextureLoader::loadData(
    std::uint16_t id
) const {
    const eld::archive::ArchiveFile& dataFile =
        archive_.get(
            std::to_string(id) + ".dat"
        );

    const eld::archive::ArchiveFile& indexFile =
        archive_.get("index.dat");

    try {
        return decoder_.decode(
            dataFile.payload,
            indexFile.payload
        );
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode texture " +
            std::to_string(id) +
            ": " +
            error.what()
        );
    }
}


const TextureResource&
TextureLoader::resource(
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
                id,
                data(id)
            )
        );

    return inserted.first->second;
}


std::optional<TextureResource>
TextureLoader::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return resource(id);
}


std::vector<std::uint16_t>
TextureLoader::listIds() const {
    std::vector<std::uint16_t> ids;
    ids.reserve(count());

    for (
        std::uint32_t candidate = 0;
        candidate <=
            std::numeric_limits<std::uint16_t>::max();
        ++candidate
    ) {
        const auto id =
            static_cast<std::uint16_t>(candidate);

        if (contains(id)) {
            ids.push_back(id);
        }
    }

    return ids;
}


bool TextureLoader::contains(
    std::uint16_t id
) const {
    return archive_.contains(
        std::to_string(id) + ".dat"
    );
}


std::size_t TextureLoader::count() const {
    const std::size_t fileCount =
        archive_.count();

    return fileCount == 0
        ? 0
        : fileCount - 1;
}

}
