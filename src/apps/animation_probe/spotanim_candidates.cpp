#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "animation/AnimationRepository.h"
#include "cache/Cache.h"
#include "cache/Index.h"
#include "definition/DefinitionRepository.h"
#include "definition/sequence/SequenceRepository.h"
#include "definition/spot_animation/SpotAnimationRepository.h"
#include "model/ModelRepository.h"

namespace {

struct FrameLocation {
    std::uint16_t archiveId = 0;
    std::size_t frameIndex = 0;
};

struct AnimationIndex {
    std::map<std::uint16_t, eld::animation::Animation> archives;
    std::map<std::uint16_t, FrameLocation> frames;
};

AnimationIndex buildAnimationIndex(
    const eld::animation::AnimationRepository& repository
) {
    AnimationIndex index;

    for (const std::uint16_t archiveId : repository.listIds()) {
        eld::animation::Animation animation = repository.get(archiveId);

        for (
            std::size_t i = 0;
            i < animation.asset.frames.size();
            ++i
        ) {
            index.frames.emplace(
                animation.asset.frames[i].id,
                FrameLocation{
                    archiveId,
                    i
                }
            );
        }

        index.archives.emplace(
            archiveId,
            std::move(animation)
        );
    }

    return index;
}

bool modelHasSkinData(
    const eld::model::ModelMesh& mesh
) {
    for (const eld::model::Vertex& vertex : mesh.vertices) {
        if (vertex.skin.has_value()) {
            return true;
        }
    }

    return false;
}

struct CandidateInfo {
    std::size_t frames = 0;
    std::size_t transforms = 0;
    bool hasTranslate = false;
    bool hasScale = false;
    bool hasAlpha = false;
    bool hasRotate = false;
    bool hasUnknown4 = false;
    std::set<std::uint16_t> archives;
};

std::optional<CandidateInfo> inspectCandidate(
    const eld::definition::SpotAnimationDefinition& spot,
    const eld::definition::SequenceRepository& sequences,
    const eld::model::ModelRepository& models,
    const AnimationIndex& animations
) {
    if (
        !spot.modelId.has_value() ||
        !spot.sequenceId.has_value()
    ) {
        return std::nullopt;
    }

    const eld::definition::SequenceDefinition* sequence =
        sequences.find(*spot.sequenceId);

    if (
        sequence == nullptr ||
        sequence->frames.empty()
    ) {
        return std::nullopt;
    }

    const std::optional<eld::model::Model> model =
        models.find(*spot.modelId);

    if (
        !model.has_value() ||
        !modelHasSkinData(model->mesh)
    ) {
        return std::nullopt;
    }

    CandidateInfo info;
    info.frames = sequence->frames.size();

    for (
        const eld::definition::SequenceFrame& sequenceFrame :
        sequence->frames
    ) {
        const auto frameLocation =
            animations.frames.find(
                sequenceFrame.primaryFrameId
            );

        if (frameLocation == animations.frames.end()) {
            return std::nullopt;
        }

        const auto archive =
            animations.archives.find(
                frameLocation->second.archiveId
            );

        if (archive == animations.archives.end()) {
            return std::nullopt;
        }

        const eld::animation::Animation& animation =
            archive->second;

        if (
            frameLocation->second.frameIndex >=
            animation.asset.frames.size()
        ) {
            return std::nullopt;
        }

        const eld::animation::AnimationFrame& frame =
            animation.asset.frames[
                frameLocation->second.frameIndex
            ];

        info.archives.insert(
            frameLocation->second.archiveId
        );

        for (
            const eld::animation::FrameTransform& transform :
            frame.transforms
        ) {
            ++info.transforms;

            if (
                transform.slot >=
                animation.asset.skeleton.slots.size()
            ) {
                continue;
            }

            const std::uint8_t type =
                animation.asset.skeleton.slots[
                    transform.slot
                ].type;

            switch (type) {
                case 1:
                    info.hasTranslate = true;
                    break;
                case 2:
                    info.hasRotate = true;
                    break;
                case 3:
                    info.hasScale = true;
                    break;
                case 4:
                    info.hasUnknown4 = true;
                    break;
                case 5:
                    info.hasAlpha = true;
                    break;
                default:
                    break;
            }
        }
    }

    return info;
}

int run(
    const std::filesystem::path& cacheRoot
) {
    eld::cache::Cache cache(cacheRoot);

    eld::definition::DefinitionRepository definitions(
        cache.open(eld::cache::IndexId::Config),
        2
    );

    eld::definition::SpotAnimationRepository spots(
        definitions.get("spotanim")
    );

    eld::definition::SequenceRepository sequences(
        definitions.get("seq")
    );

    eld::model::ModelRepository models(
        cache.open(eld::cache::IndexId::Models)
    );

    eld::animation::AnimationRepository animationRepository(
        cache.open(eld::cache::IndexId::Animations)
    );

    std::cout
        << "Building global animation index...\n";

    const AnimationIndex animations =
        buildAnimationIndex(animationRepository);

    std::cout
        << "\nSafe geometry-test SpotAnims\n"
        << "============================\n"
        << "Criteria: model+sequence+skins, all frames resolve,\n"
        << "uses translation and/or scale, NO active rotation, NO type4.\n\n";

    std::size_t printed = 0;

    for (
        const eld::definition::SpotAnimationDefinition& spot :
        spots.list()
    ) {
        const std::optional<CandidateInfo> info =
            inspectCandidate(
                spot,
                sequences,
                models,
                animations
            );

        if (!info.has_value()) {
            continue;
        }

        if (
            !info->hasRotate ||
            info->hasUnknown4 ||
            (
                !info->hasTranslate &&
                !info->hasScale
            )
        ) {
            continue;
        }

        std::cout
            << "SpotAnim "
            << spot.id
            << "  model="
            << *spot.modelId
            << "  seq="
            << *spot.sequenceId
            << "  frames="
            << info->frames
            << "  transforms="
            << info->transforms
            << "  archives="
            << info->archives.size()
            << "  [";

        if (info->hasTranslate) {
            std::cout << "translate ";
        }

        if (info->hasScale) {
            std::cout << "scale ";
        }

        if (info->hasAlpha) {
            std::cout << "alpha ";
        }

        std::cout << "]\n";

        ++printed;

        if (printed >= 12) {
            break;
        }
    }

    std::cout
        << "\nTry any of those with:\n"
        << "  ./build/bin/animation_visual_probe ./cache <spotanim-id>\n";

    return printed == 0 ? 1 : 0;
}

}

int main(
    int argc,
    char** argv
) {
    if (argc != 2) {
        std::cerr
            << "usage: "
            << (argc > 0 ? argv[0] : "animation_spotanim_candidates")
            << " <cache-root>\n";
        return 1;
    }

    try {
        return run(
            std::filesystem::path(argv[1])
        );
    }
    catch (const std::exception& e) {
        std::cerr
            << "animation_spotanim_candidates failed: "
            << e.what()
            << '\n';
        return 1;
    }
}
