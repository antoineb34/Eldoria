#include "TextureRepository.h"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

#include "archive/ArchiveParser.h"

namespace eld::texture {

namespace {

constexpr std::uint16_t TextureArchiveId = 6;

}

eld::archive::Archive TextureRepository::loadArchive(
    const eld::cache::Store& store
) {
    const eld::cache::File cacheFile =
        store.get(
            TextureArchiveId
        );

    eld::archive::ArchiveParser parser;

    std::optional<eld::archive::Archive> archive =
        parser.parse(
            cacheFile.getBytes()
        );

    if (!archive.has_value()) {
        throw std::runtime_error(
            "Failed to parse texture archive"
        );
    }

    return std::move(*archive);
}

TextureRepository::TextureRepository(
    eld::cache::Store store
)
    : archive_(
          loadArchive(store)
      ) {
}

const eld::archive::ArchiveFile&
TextureRepository::getIndexFile() const {
    return archive_.get(
        "index.dat"
    );
}

const eld::archive::ArchiveFile&
TextureRepository::getDataFile(
    std::uint16_t id
) const {
    return archive_.get(
        std::to_string(id) +
        ".dat"
    );
}

Texture TextureRepository::get(
    std::uint16_t id
) const {
    eld::image::IndexedImageFile file =
        getFile(id);

    eld::image::IndexedImageSourceMap sourceMap;

    eld::image::Image image =
        decoder_.decode(
            file,
            sourceMap
        );

    return Texture{
        .id = id,
        .file = std::move(file),
        .image = std::move(image),
        .sourceMap = std::move(sourceMap)
    };
}

std::optional<Texture> TextureRepository::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return get(id);
}

eld::image::IndexedImageFile TextureRepository::getFile(
    std::uint16_t id
) const {
    const eld::archive::ArchiveFile& indexFile =
        getIndexFile();

    const eld::archive::ArchiveFile& dataFile =
        getDataFile(id);

    std::optional<eld::image::IndexedImageFile> file =
        parser_.parse(
            dataFile.payload,
            indexFile.payload
        );

    if (!file.has_value()) {
        throw std::runtime_error(
            "Failed to parse texture " +
            std::to_string(id)
        );
    }

    return std::move(*file);
}

eld::image::Image TextureRepository::getImage(
    std::uint16_t id
) const {
    const eld::image::IndexedImageFile file =
        getFile(id);

    return decoder_.decode(
        file
    );
}

std::vector<std::uint16_t>
TextureRepository::listIds() const {
    std::vector<std::uint16_t> ids;

    if (archive_.count() > 0) {
        ids.reserve(
            archive_.count() - 1
        );
    }

    for (
        std::uint32_t candidate = 0;
        candidate <=
            std::numeric_limits<std::uint16_t>::max();
        candidate++
    ) {
        const std::uint16_t id =
            static_cast<std::uint16_t>(
                candidate
            );

        if (contains(id)) {
            ids.push_back(id);
        }
    }

    return ids;
}

std::vector<std::uint16_t>
TextureRepository::filterIds(
    const TexturePredicate& predicate
) const {
    const std::vector<std::uint16_t> ids =
        listIds();

    std::vector<std::uint16_t> matchingIds;

    for (const std::uint16_t id : ids) {
        const Texture texture =
            get(id);

        if (predicate(texture)) {
            matchingIds.push_back(id);
        }
    }

    return matchingIds;
}

bool TextureRepository::contains(
    std::uint16_t id
) const {
    return archive_.contains(
        std::to_string(id) +
        ".dat"
    );
}

std::size_t TextureRepository::count() const {
    return listIds().size();
}

std::size_t TextureRepository::count(
    const TexturePredicate& predicate
) const {
    const std::vector<std::uint16_t> ids =
        listIds();

    std::size_t matchingCount = 0;

    for (const std::uint16_t id : ids) {
        if (predicate(get(id))) {
            matchingCount++;
        }
    }

    return matchingCount;
}

}
