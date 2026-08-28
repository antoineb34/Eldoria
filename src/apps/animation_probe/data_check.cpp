#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <limits>
#include <unordered_set>
#include <vector>

#include "animation/AnimationRepository.h"
#include "cache/Cache.h"
#include "cache/Index.h"

namespace {

int run(
    const std::filesystem::path& cacheRoot
) {
    eld::cache::Cache cache(
        cacheRoot
    );

    eld::animation::AnimationRepository repository(
        cache.open(
            eld::cache::IndexId::Animations
        )
    );

    const std::vector<std::uint16_t> ids =
        repository.listIds();

    std::size_t passed = 0;
    std::size_t failed = 0;
    std::size_t totalFrames = 0;
    std::size_t totalSkeletonSlots = 0;
    std::size_t totalTransforms = 0;

    std::unordered_set<std::uint16_t> frameIds;

    std::uint16_t minFrame =
        std::numeric_limits<std::uint16_t>::max();

    std::uint16_t maxFrame = 0;

    for (
        const std::uint16_t id :
        ids
    ) {
        try {
            const eld::animation::Animation animation =
                repository.get(
                    id
                );

            ++passed;

            totalFrames +=
                animation.asset.frames.size();

            totalSkeletonSlots +=
                animation.asset.skeleton.slots.size();

            for (
                const eld::animation::AnimationFrame& frame :
                animation.asset.frames
            ) {
                frameIds.insert(
                    frame.id
                );

                if (
                    frame.id <
                    minFrame
                ) {
                    minFrame =
                        frame.id;
                }

                if (
                    frame.id >
                    maxFrame
                ) {
                    maxFrame =
                        frame.id;
                }

                totalTransforms +=
                    frame.transforms.size();
            }
        }
        catch (
            const std::exception& exception
        ) {
            ++failed;

            std::cerr
                << "FAIL archive "
                << id
                << ": "
                << exception.what()
                << '\n';
        }
    }

    std::cout
        << "Production animation data check\n"
        << "===============================\n"
        << "archives:              "
        << ids.size()
        << '\n'
        << "PASS:                  "
        << passed
        << '\n'
        << "FAIL:                  "
        << failed
        << '\n'
        << "decoded frames:        "
        << totalFrames
        << '\n'
        << "unique frame ids:      "
        << frameIds.size()
        << '\n';

    if (
        !frameIds.empty()
    ) {
        std::cout
            << "frame id range:        "
            << minFrame
            << " .. "
            << maxFrame
            << '\n';
    }

    std::cout
        << "skeleton slots:        "
        << totalSkeletonSlots
        << '\n'
        << "active transforms:     "
        << totalTransforms
        << '\n';

    if (
        failed ==
        0
    ) {
        std::cout
            << "\nPRODUCTION ANIMATION DATA LOAD: PASS\n";

        return 0;
    }

    std::cout
        << "\nPRODUCTION ANIMATION DATA LOAD: FAIL\n";

    return 1;
}

}

int main(
    int argc,
    char** argv
) {
    if (
        argc !=
        2
    ) {
        std::cerr
            << "usage: "
            << (
                argc > 0
                    ? argv[0]
                    : "animation_data_check"
            )
            << " <cache-root>\n";

        return 1;
    }

    try {
        return run(
            std::filesystem::path(
                argv[1]
            )
        );
    }
    catch (
        const std::exception& exception
    ) {
        std::cerr
            << "animation_data_check failed: "
            << exception.what()
            << '\n';

        return 1;
    }
}
