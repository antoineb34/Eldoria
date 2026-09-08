#include "title/TitleLoader.h"

#include <exception>
#include <stdexcept>
#include <string>

namespace eld::title {

TitleLoader::TitleLoader(
    const eld::cache::Cache& cache
)
    : archive_(
          eld::archive::load(
              cache.open(Index),
              ArchiveId
          )
      ) {
}


const eld::image::ImageData&
TitleLoader::data(
    std::string_view name
) const {
    const std::string key(name);

    const auto existing =
        dataCache_.find(key);

    if (existing != dataCache_.end()) {
        return existing->second;
    }

    try {
        const auto inserted =
            dataCache_.emplace(
                key,
                decoder_.decode(
                    archive_.get(name).payload
                )
            );

        return inserted.first->second;
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode title image " +
            key +
            ": " +
            error.what()
        );
    }
}


std::optional<eld::image::ImageData>
TitleLoader::find(
    std::string_view name
) const {
    if (!contains(name)) {
        return std::nullopt;
    }

    return data(name);
}


bool TitleLoader::contains(
    std::string_view name
) const {
    return archive_.contains(name);
}

}
