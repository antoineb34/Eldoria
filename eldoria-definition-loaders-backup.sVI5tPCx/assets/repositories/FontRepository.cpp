#include "repositories/FontRepository.h"

#include <exception>
#include <stdexcept>
#include <string>

#include "archive/ArchiveHashes.h"

namespace eld::font {

FontRepository::FontRepository(
    const eld::cache::Cache& cache
)
    : archive_(
          eld::archive::load(
              cache.open(Index),
              ArchiveId
          )
      ) {
}


Font FontRepository::get(
    std::uint16_t id
) const {
    const eld::archive::ArchiveFile& dataFile =
        archive_.get(id);

    if (
        dataFile.nameHash ==
        eld::archive::hashName(IndexFile)
    ) {
        throw std::out_of_range(
            "Font does not exist"
        );
    }

    const eld::archive::ArchiveFile& indexFile =
        archive_.get(IndexFile);

    try {
        Font font =
            decoder_.decode(
                dataFile.payload,
                indexFile.payload
            );

        font.id = id;

        const std::optional<std::string_view> name =
            eld::archive::findName(
                dataFile.nameHash
            );

        if (name.has_value()) {
            font.name =
                std::string(*name);
        }

        return font;
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode font " +
            std::to_string(id) +
            ": " +
            error.what()
        );
    }
}


std::optional<Font> FontRepository::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return get(id);
}


std::vector<std::uint16_t>
FontRepository::listIds() const {
    const std::uint32_t indexHash =
        eld::archive::hashName(IndexFile);

    std::vector<std::uint16_t> ids;

    ids.reserve(
        archive_.count()
    );

    for (
        const eld::archive::ArchiveFile& file :
        archive_.list()
    ) {
        if (file.nameHash == indexHash) {
            continue;
        }

        ids.push_back(
            file.id
        );
    }

    return ids;
}


bool FontRepository::contains(
    std::uint16_t id
) const {
    const eld::archive::ArchiveFile* file =
        archive_.find(id);

    return
        file != nullptr &&
        file->nameHash !=
            eld::archive::hashName(IndexFile);
}


std::size_t FontRepository::count() const {
    return listIds().size();
}

}
