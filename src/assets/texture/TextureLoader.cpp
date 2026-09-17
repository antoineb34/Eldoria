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
              cache.open(CacheIndex),
              TextureArchiveId
          )
      ) {
}


const eld::image::ImageData&
TextureLoader::get(
    std::uint16_t id
) const {
    const auto existing =
        textureCache_.find(id);

    if (existing != textureCache_.end()) {
        return existing->second;
    }

    const auto inserted =
        textureCache_.emplace(
            id,
            load(id)
        );

    return inserted.first->second;
}


const eld::image::ImageData*
TextureLoader::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return nullptr;
    }

    return &get(id);
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
            static_cast<std::uint16_t>(
                candidate
            );

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
        fileName(id)
    );
}


std::size_t TextureLoader::count() const {
    const std::size_t fileCount =
        archive_.count();

    return fileCount == 0
        ? 0
        : fileCount - 1;
}


std::string TextureLoader::fileName(
    std::uint16_t id
) const {
    return std::to_string(id) +
        std::string(TextureFileExtension);
}


eld::image::ImageData
TextureLoader::load(
    std::uint16_t id
) const {
    const eld::archive::ArchiveFile& dataFile =
        archive_.get(
            fileName(id)
        );

    const eld::archive::ArchiveFile& indexFile =
        archive_.get(
            IndexFileName
        );

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

}
