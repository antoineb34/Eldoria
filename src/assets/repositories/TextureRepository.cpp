#include "repositories/TextureRepository.h"

#include <exception>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>


namespace eld::texture {

TextureRepository::TextureRepository(
    const eld::cache::Cache& cache
)
    : archive_(
          eld::archive::load(
              cache.open(Index),
              ArchiveId
          )
      ) {
}


Texture TextureRepository::get(
    std::uint16_t id
) const {
    const eld::archive::ArchiveFile& data =
        archive_.get(
            std::to_string(id) + ".dat"
        );

    const eld::archive::ArchiveFile& index =
        archive_.get("index.dat");

    try {
        return Texture{
            .id = id,
            .image = decoder_.decode(
                data.payload,
                index.payload
            )
        };
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode texture " +
            std::to_string(id) +
            ": " +
            error.what()
        );
    }
}


std::optional<Texture> TextureRepository::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return get(id);
}


std::vector<std::uint16_t>
TextureRepository::listIds() const {
    std::vector<std::uint16_t> ids;

    ids.reserve(count());

    for (
        std::uint32_t candidate = 0;
        candidate <=
            std::numeric_limits<std::uint16_t>::max();
        ++candidate
    ) {
        const auto id =
            static_cast<std::uint16_t>(candidate);

        if (contains(id)) {
            ids.push_back(id);
        }
    }

    return ids;
}


bool TextureRepository::contains(
    std::uint16_t id
) const {
    return archive_.contains(
        std::to_string(id) + ".dat"
    );
}


std::size_t TextureRepository::count() const {
    const std::size_t fileCount =
        archive_.count();

    return fileCount > 0
        ? fileCount - 1
        : 0;
}

}
