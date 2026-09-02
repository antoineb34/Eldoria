#include "AnimationRepository.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace eld::animation {

AnimationRepository::AnimationRepository(
    eld::cache::Store store
)
    : store_(
        std::move(
            store
        )
    ) {
}

Animation AnimationRepository::get(
    std::uint16_t id
) const {
    AnimationFile file =
        getFile(
            id
        );

    AnimationAsset asset =
        decoder_.decode(
            file
        );

    return Animation{
        .id = id,
        .file = std::move(file),
        .asset = std::move(asset)
    };
}

std::optional<Animation>
AnimationRepository::find(
    std::uint16_t id
) const {
    if (
        !contains(
            id
        )
    ) {
        return std::nullopt;
    }

    return get(
        id
    );
}

AnimationFile AnimationRepository::getFile(
    std::uint16_t id
) const {
    const eld::cache::File cacheFile =
        store_.get(
            id
        );

    std::optional<AnimationFile> file =
        parser_.parse(
            cacheFile.getBytes()
        );

    if (
        !file.has_value()
    ) {
        throw std::runtime_error(
            "Failed to parse animation archive " +
            std::to_string(
                id
            )
        );
    }

    return std::move(
        *file
    );
}

AnimationAsset AnimationRepository::getAsset(
    std::uint16_t id
) const {
    const AnimationFile file =
        getFile(
            id
        );

    return decoder_.decode(
        file
    );
}

std::vector<std::uint16_t>
AnimationRepository::listIds() const {
    const std::vector<eld::cache::FileEntry> entries =
        store_.list();

    std::vector<std::uint16_t> ids;

    ids.reserve(
        entries.size()
    );

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

std::vector<std::uint16_t>
AnimationRepository::filterIds(
    const AnimationPredicate& predicate
) const {
    const std::vector<std::uint16_t> ids =
        listIds();

    std::vector<std::uint16_t> matchingIds;

    for (
        const std::uint16_t id :
        ids
    ) {
        const Animation animation =
            get(
                id
            );

        if (
            predicate(
                animation
            )
        ) {
            matchingIds.push_back(
                id
            );
        }
    }

    return matchingIds;
}

bool AnimationRepository::contains(
    std::uint16_t id
) const {
    return store_.contains(
        id
    );
}

std::size_t AnimationRepository::count() const {
    return store_.count();
}

std::size_t AnimationRepository::count(
    const AnimationPredicate& predicate
) const {
    const std::vector<std::uint16_t> ids =
        listIds();

    std::size_t matchingCount = 0;

    for (
        const std::uint16_t id :
        ids
    ) {
        if (
            predicate(
                get(
                    id
                )
            )
        ) {
            ++matchingCount;
        }
    }

    return matchingCount;
}

}
