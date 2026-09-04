#include "repositories/SpriteRepository.h"

#include <exception>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>


namespace eld::sprite {

SpriteRepository::SpriteRepository(
    const eld::cache::Cache& cache,
    std::uint16_t archiveId
)
    : archive_(
          eld::archive::load(
              cache.open(Index),
              archiveId
          )
      ) {
}


const eld::archive::ArchiveFile&
SpriteRepository::getIndexFile() const {
    return archive_.get("index.dat");
}


const eld::archive::ArchiveFile&
SpriteRepository::getDataFile(
    std::string_view groupName
) const {
    return archive_.get(groupName);
}


const eld::archive::ArchiveFile&
SpriteRepository::getDataFile(
    std::uint16_t fileId
) const {
    return archive_.get(fileId);
}


Sprite SpriteRepository::decodeSprite(
    const eld::archive::ArchiveFile& dataFile,
    std::string_view groupName,
    std::uint16_t frameId
) const {
    try {
        eld::image::Image image =
            decoder_.decode(
                dataFile.payload,
                getIndexFile().payload,
                frameId
            );

        return Sprite{
            .groupName = std::string(groupName),
            .frameId = frameId,
            .image = std::move(image)
        };
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode sprite frame " +
            std::to_string(frameId) +
            ": " +
            error.what()
        );
    }
}


Sprite SpriteRepository::get(
    std::string_view groupName,
    std::uint16_t frameId
) const {
    return decodeSprite(
        getDataFile(groupName),
        groupName,
        frameId
    );
}


Sprite SpriteRepository::get(
    std::uint16_t fileId,
    std::uint16_t frameId
) const {
    const eld::archive::ArchiveFile& file =
        getDataFile(fileId);

    return decodeSprite(
        file,
        "file-" + std::to_string(fileId),
        frameId
    );
}


std::optional<Sprite> SpriteRepository::find(
    std::string_view groupName,
    std::uint16_t frameId
) const {
    try {
        return get(groupName, frameId);
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}


std::optional<Sprite> SpriteRepository::find(
    std::uint16_t fileId,
    std::uint16_t frameId
) const {
    try {
        return get(fileId, frameId);
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}


bool SpriteRepository::contains(
    std::string_view groupName,
    std::uint16_t frameId
) const {
    return find(groupName, frameId).has_value();
}


bool SpriteRepository::contains(
    std::uint16_t fileId,
    std::uint16_t frameId
) const {
    return find(fileId, frameId).has_value();
}


std::vector<std::uint16_t>
SpriteRepository::listFrameIds(
    std::string_view groupName
) const {
    std::vector<std::uint16_t> ids;

    for (
        std::uint32_t candidate = 0;
        candidate <=
            std::numeric_limits<std::uint16_t>::max();
        ++candidate
    ) {
        const auto frameId =
            static_cast<std::uint16_t>(candidate);

        if (!contains(groupName, frameId)) {
            break;
        }

        ids.push_back(frameId);
    }

    return ids;
}


std::vector<std::uint16_t>
SpriteRepository::listFrameIds(
    std::uint16_t fileId
) const {
    std::vector<std::uint16_t> ids;

    for (
        std::uint32_t candidate = 0;
        candidate <=
            std::numeric_limits<std::uint16_t>::max();
        ++candidate
    ) {
        const auto frameId =
            static_cast<std::uint16_t>(candidate);

        if (!contains(fileId, frameId)) {
            break;
        }

        ids.push_back(frameId);
    }

    return ids;
}


std::size_t SpriteRepository::countFrames(
    std::string_view groupName
) const {
    return listFrameIds(groupName).size();
}


std::size_t SpriteRepository::countFrames(
    std::uint16_t fileId
) const {
    return listFrameIds(fileId).size();
}

}
