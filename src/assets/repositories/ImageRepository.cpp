#include "repositories/ImageRepository.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>


namespace eld::image {

ImageRepository::ImageRepository(
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


Image ImageRepository::get(
    std::string_view fileName
) const {
    const eld::archive::ArchiveFile& file =
        archive_.get(fileName);

    try {
        return decoder_.decode(
            file.payload
        );
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode image " +
            std::string(fileName) +
            ": " +
            error.what()
        );
    }
}


std::optional<Image> ImageRepository::find(
    std::string_view fileName
) const {
    if (!contains(fileName)) {
        return std::nullopt;
    }

    return get(fileName);
}


bool ImageRepository::contains(
    std::string_view fileName
) const {
    return archive_.contains(fileName);
}

}
