#include "JpegRepository.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

#include "archive/ArchiveParser.h"

namespace eld::image {

eld::archive::Archive JpegRepository::loadArchive(
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
            "Failed to parse JPEG archive " +
            std::to_string(archiveId)
        );
    }

    return std::move(*archive);
}

JpegRepository::JpegRepository(
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

Image JpegRepository::get(
    std::string_view fileName
) const {
    const eld::archive::ArchiveFile& file =
        archive_.get(
            fileName
        );

    return decoder_.decode(
        file.payload
    );
}

std::optional<Image> JpegRepository::find(
    std::string_view fileName
) const {
    if (!contains(fileName)) {
        return std::nullopt;
    }

    try {
        return get(
            fileName
        );
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

bool JpegRepository::contains(
    std::string_view fileName
) const {
    return archive_.contains(
        fileName
    );
}

}
