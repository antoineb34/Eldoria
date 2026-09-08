#include "animation/AnimationLoader.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

namespace eld::animation {

AnimationLoader::AnimationLoader(
    const eld::cache::Cache& cache
)
    : store_(
          cache.open(Index)
      ) {
}


const AnimationData& AnimationLoader::data(
    std::uint16_t id
) const {
    const auto existing =
        dataCache_.find(id);

    if (existing != dataCache_.end()) {
        return existing->second;
    }

    const auto inserted =
        dataCache_.emplace(
            id,
            loadData(id)
        );

    return inserted.first->second;
}


AnimationData AnimationLoader::loadData(
    std::uint16_t id
) const {
    const eld::cache::File file =
        store_.get(id);

    try {
        AnimationData animation =
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


std::optional<AnimationData>
AnimationLoader::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return data(id);
}


std::vector<std::uint16_t>
AnimationLoader::listIds() const {
    const std::vector<eld::cache::FileEntry> entries =
        store_.list();

    std::vector<std::uint16_t> ids;
    ids.reserve(entries.size());

    for (
        const eld::cache::FileEntry& entry :
        entries
    ) {
        ids.push_back(entry.fileId);
    }

    return ids;
}


bool AnimationLoader::contains(
    std::uint16_t id
) const {
    return store_.contains(id);
}


std::size_t AnimationLoader::count() const {
    return store_.count();
}

}
