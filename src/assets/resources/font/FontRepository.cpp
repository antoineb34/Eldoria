#include "FontRepository.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

#include "archive/ArchiveParser.h"

namespace eld::font {

eld::archive::Archive FontRepository::loadArchive(
    const eld::cache::Store& store,
    std::uint16_t archiveId
) {
    const eld::cache::File cacheFile =
        store.get(
            archiveId
        );

    eld::archive::ArchiveParser parser;

    std::optional<eld::archive::Archive> archive =
        parser.parse(
            cacheFile.getBytes()
        );

    if (!archive.has_value()) {
        throw std::runtime_error(
            "Failed to parse font archive"
        );
    }

    return std::move(*archive);
}

FontRepository::FontRepository(
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

Font FontRepository::get(
    std::string_view name
) const {
    const eld::archive::ArchiveFile& indexFile =
        archive_.get(
            "index.dat"
        );

    const eld::archive::ArchiveFile& dataFile =
        archive_.get(
            name
        );

    try {
        return decoder_.decode(
            dataFile.payload,
            indexFile.payload,
            std::string(name)
        );
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode font " +
            std::string(name) +
            ": " +
            error.what()
        );
    }
}

std::optional<Font> FontRepository::find(
    std::string_view name
) const {
    if (!contains(name)) {
        return std::nullopt;
    }

    try {
        return get(name);
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

bool FontRepository::contains(
    std::string_view name
) const {
    return
        archive_.contains("index.dat") &&
        archive_.contains(name);
}

}
