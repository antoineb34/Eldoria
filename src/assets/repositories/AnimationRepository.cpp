#include "repositories/AnimationRepository.h"

#include <exception>
#include <stdexcept>
#include <string>

namespace eld::animation {

AnimationRepository::AnimationRepository(
    const eld::cache::Cache& cache
)
    : store_(
          cache.open(Index)
      ) {
}


Animation AnimationRepository::get(
    std::uint16_t id
) const {
    const eld::cache::File file =
        store_.get(id);

    try {
        Animation animation =
            decoder_.decode(
                file.getBytes()
            );

        animation.id = id;

        return animation;
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode animation " +
            std::to_string(id) +
            ": " +
            error.what()
        );
    }
}


std::optional<Animation>
AnimationRepository::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return get(id);
}


std::vector<std::uint16_t>
AnimationRepository::listIds() const {
    const std::vector<eld::cache::FileEntry> entries =
        store_.list();

    std::vector<std::uint16_t> ids;
    ids.reserve(entries.size());

    for (
        const eld::cache::FileEntry& entry :
        entries
    ) {
        ids.push_back(
            entry.fileId
        );
    }

    return ids;
}


bool AnimationRepository::contains(
    std::uint16_t id
) const {
    return store_.contains(id);
}


std::size_t AnimationRepository::count() const {
    return store_.count();
}

}
