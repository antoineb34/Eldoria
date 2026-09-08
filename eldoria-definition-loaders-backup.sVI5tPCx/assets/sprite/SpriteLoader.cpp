#include "sprite/SpriteLoader.h"

#include <string>
#include <utility>

namespace eld::sprite {

namespace {

std::string fileKey(
    std::uint16_t fileId
) {
    return "file-" +
        std::to_string(fileId);
}

}


SpriteLoader::SpriteLoader(
    const eld::cache::Cache& cache,
    SpriteArchive archive
)
    : archive_(
          eld::archive::load(
              cache.open(Index),
              static_cast<std::uint16_t>(archive)
          )
      ) {
}


const eld::archive::ArchiveFile&
SpriteLoader::indexFile() const {
    return archive_.get("index.dat");
}


const eld::archive::ArchiveFile&
SpriteLoader::dataFile(
    std::string_view groupName
) const {
    return archive_.get(groupName);
}


const eld::archive::ArchiveFile&
SpriteLoader::dataFile(
    std::uint16_t fileId
) const {
    return archive_.get(fileId);
}


const eld::image::ImageData&
SpriteLoader::cachedData(
    Key key,
    const eld::archive::ArchiveFile& file
) const {
    const auto existing =
        dataCache_.find(key);

    if (existing != dataCache_.end()) {
        return existing->second;
    }

    const std::uint16_t frameId =
        key.second;

    const auto inserted =
        dataCache_.emplace(
            std::move(key),
            decoder_.decode(
                file.payload,
                indexFile().payload,
                frameId
            )
        );

    return inserted.first->second;
}


const eld::image::ImageData&
SpriteLoader::data(
    std::string_view groupName,
    std::uint16_t frameId
) const {
    return cachedData(
        Key{
            std::string(groupName),
            frameId
        },
        dataFile(groupName)
    );
}


const eld::image::ImageData&
SpriteLoader::data(
    std::uint16_t fileId,
    std::uint16_t frameId
) const {
    return cachedData(
        Key{
            fileKey(fileId),
            frameId
        },
        dataFile(fileId)
    );
}


const SpriteResource&
SpriteLoader::cachedResource(
    Key key,
    const eld::image::ImageData& dataValue
) const {
    const auto existing =
        resourceCache_.find(key);

    if (existing != resourceCache_.end()) {
        return existing->second;
    }

    const std::string groupName =
        key.first;

    const std::uint16_t frameId =
        key.second;

    const auto inserted =
        resourceCache_.emplace(
            std::move(key),
            assembler_.assemble(
                groupName,
                frameId,
                dataValue
            )
        );

    return inserted.first->second;
}


const SpriteResource&
SpriteLoader::resource(
    std::string_view groupName,
    std::uint16_t frameId
) const {
    const Key key{
        std::string(groupName),
        frameId
    };

    return cachedResource(
        key,
        data(groupName, frameId)
    );
}


const SpriteResource&
SpriteLoader::resource(
    std::uint16_t fileId,
    std::uint16_t frameId
) const {
    const Key key{
        fileKey(fileId),
        frameId
    };

    return cachedResource(
        key,
        data(fileId, frameId)
    );
}


std::optional<SpriteResource>
SpriteLoader::find(
    std::string_view groupName,
    std::uint16_t frameId
) const {
    if (!contains(groupName, frameId)) {
        return std::nullopt;
    }

    return resource(groupName, frameId);
}


std::optional<SpriteResource>
SpriteLoader::find(
    std::uint16_t fileId,
    std::uint16_t frameId
) const {
    if (!contains(fileId, frameId)) {
        return std::nullopt;
    }

    return resource(fileId, frameId);
}


std::size_t SpriteLoader::countFrames(
    std::string_view groupName
) const {
    const eld::archive::ArchiveFile* data =
        archive_.find(groupName);

    const eld::archive::ArchiveFile* index =
        archive_.find("index.dat");

    if (data == nullptr || index == nullptr) {
        return 0;
    }

    return decoder_.frameCount(
        data->payload,
        index->payload
    );
}


std::size_t SpriteLoader::countFrames(
    std::uint16_t fileId
) const {
    const eld::archive::ArchiveFile* data =
        archive_.find(fileId);

    const eld::archive::ArchiveFile* index =
        archive_.find("index.dat");

    if (data == nullptr || index == nullptr) {
        return 0;
    }

    return decoder_.frameCount(
        data->payload,
        index->payload
    );
}


bool SpriteLoader::contains(
    std::string_view groupName,
    std::uint16_t frameId
) const {
    return frameId <
        countFrames(groupName);
}


bool SpriteLoader::contains(
    std::uint16_t fileId,
    std::uint16_t frameId
) const {
    return frameId <
        countFrames(fileId);
}


std::vector<std::uint16_t>
SpriteLoader::listFrameIds(
    std::string_view groupName
) const {
    const std::size_t frameCount =
        countFrames(groupName);

    std::vector<std::uint16_t> ids;
    ids.reserve(frameCount);

    for (
        std::size_t id = 0;
        id < frameCount;
        ++id
    ) {
        ids.push_back(
            static_cast<std::uint16_t>(id)
        );
    }

    return ids;
}


std::vector<std::uint16_t>
SpriteLoader::listFrameIds(
    std::uint16_t fileId
) const {
    const std::size_t frameCount =
        countFrames(fileId);

    std::vector<std::uint16_t> ids;
    ids.reserve(frameCount);

    for (
        std::size_t id = 0;
        id < frameCount;
        ++id
    ) {
        ids.push_back(
            static_cast<std::uint16_t>(id)
        );
    }

    return ids;
}

}
