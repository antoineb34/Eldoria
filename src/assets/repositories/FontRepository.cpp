#include "repositories/FontRepository.h"

#include <exception>
#include <stdexcept>
#include <string>

namespace eld::font {

FontRepository::FontRepository(
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


Font FontRepository::get(
    std::string_view name
) const {
    const eld::archive::ArchiveFile& indexFile =
        archive_.get(
            IndexFile
        );

    const eld::archive::ArchiveFile& dataFile =
        archive_.get(
            name
        );

    try {
        Font font =
            decoder_.decode(
                dataFile.payload,
                indexFile.payload
            );

        font.name =
            std::string(name);

        return font;
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

    return get(name);
}


bool FontRepository::contains(
    std::string_view name
) const {
    return
        archive_.contains(
            IndexFile
        ) &&
        archive_.contains(
            name
        );
}

}
