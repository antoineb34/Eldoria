#include "SpriteRepository.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>
#include <limits>
#include <vector>

#include "archive/ArchiveParser.h"

namespace eld::sprite {

eld::archive::Archive SpriteRepository::loadArchive(
    const eld::cache::Store& store,
    std::uint16_t archiveId
) {
    const eld::cache::File cacheFile =
        store.get(archiveId);

    eld::archive::ArchiveParser parser;

    std::optional<eld::archive::Archive> archive =
        parser.parse(
            cacheFile.getBytes()
        );

    if (!archive.has_value()) {
        throw std::runtime_error(
            "Failed to parse sprite archive " +
            std::to_string(archiveId)
        );
    }

    return std::move(*archive);
}

SpriteRepository::SpriteRepository(
    eld::cache::Store store,
    std::uint16_t archiveId
)
    : archive_(
          loadArchive(
              store,
              archiveId
          )
      ) {
}

const eld::archive::ArchiveFile&
SpriteRepository::getIndexFile() const {
    return archive_.get(
        "index.dat"
    );
}

const eld::archive::ArchiveFile&
SpriteRepository::getDataFile(
    std::string_view groupName
) const {
    return archive_.get(
        groupName
    );
}

eld::image::IndexedImageFile
SpriteRepository::getFile(
    std::string_view groupName,
    std::uint16_t frameId
) const {
    const eld::archive::ArchiveFile& indexFile =
        getIndexFile();

    const eld::archive::ArchiveFile& dataFile =
        getDataFile(
            groupName
        );

    std::optional<eld::image::IndexedImageFile> file =
        parser_.parse(
            dataFile.payload,
            indexFile.payload,
            frameId
        );

    if (!file.has_value()) {
        throw std::runtime_error(
            "Failed to parse sprite " +
            std::string(groupName) +
            " frame " +
            std::to_string(frameId)
        );
    }

    return std::move(*file);
}

eld::image::Image SpriteRepository::getImage(
    std::string_view groupName,
    std::uint16_t frameId
) const {
    const eld::image::IndexedImageFile file =
        getFile(
            groupName,
            frameId
        );

    return decoder_.decode(
        file
    );
}

Sprite SpriteRepository::get(
    std::string_view groupName,
    std::uint16_t frameId
) const {
    eld::image::IndexedImageFile file =
        getFile(
            groupName,
            frameId
        );

    eld::image::IndexedImageSourceMap sourceMap;

    eld::image::Image image =
        decoder_.decode(
            file,
            sourceMap
        );

    return Sprite{
        .groupName = std::string(groupName),
        .frameId = frameId,
        .file = std::move(file),
        .image = std::move(image),
        .sourceMap = std::move(sourceMap)
    };
}

std::optional<Sprite> SpriteRepository::find(
    std::string_view groupName,
    std::uint16_t frameId
) const {
    if (
        !contains(
            groupName,
            frameId
        )
    ) {
        return std::nullopt;
    }

    try {
        return get(
            groupName,
            frameId
        );
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

bool SpriteRepository::contains(
    std::string_view groupName,
    std::uint16_t frameId
) const {
    const eld::archive::ArchiveFile* indexFile =
        archive_.find(
            "index.dat"
        );

    const eld::archive::ArchiveFile* dataFile =
        archive_.find(
            groupName
        );

    if (
        indexFile == nullptr ||
        dataFile == nullptr
    ) {
        return false;
    }

    return parser_.parse(
        dataFile->payload,
        indexFile->payload,
        frameId
    ).has_value();
}

std::vector<std::uint16_t>
SpriteRepository::listFrameIds(
    std::string_view groupName
) const {
    std::vector<std::uint16_t> frameIds;

    for (
        std::uint32_t candidate = 0;
        candidate <=
            std::numeric_limits<std::uint16_t>::max();
        candidate++
    ) {
        const std::uint16_t frameId =
            static_cast<std::uint16_t>(
                candidate
            );

        if (
            !contains(
                groupName,
                frameId
            )
        ) {
            break;
        }

        frameIds.push_back(
            frameId
        );
    }

    return frameIds;
}

std::size_t SpriteRepository::countFrames(
    std::string_view groupName
) const {
    return listFrameIds(
        groupName
    ).size();
}

}
