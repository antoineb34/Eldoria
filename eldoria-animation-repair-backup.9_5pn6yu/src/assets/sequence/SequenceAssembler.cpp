#include "sequence/SequenceAssembler.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace eld::sequence {

SequenceResource SequenceAssembler::assemble(
    SequenceData data,
    const eld::animation::AnimationPipeline& animations
) const {
    SequenceResource resource;

    static_cast<SequenceData&>(resource) =
        std::move(data);

    resource.resolvedFrames.reserve(
        resource.frames.size()
    );

    for (
        const SequenceFrameData& frame :
        resource.frames
    ) {
        const auto primary =
            animations.findFrame(
                frame.primaryFrameId
            );

        if (!primary.has_value()) {
            throw std::runtime_error(
                "Sequence " +
                std::to_string(resource.id) +
                " references missing primary frame " +
                std::to_string(frame.primaryFrameId)
            );
        }

        std::optional<ResolvedAnimationFrame> secondary;

        if (frame.secondaryFrameId.has_value()) {
            const auto resolvedSecondary =
                animations.findFrame(
                    *frame.secondaryFrameId
                );

            if (!resolvedSecondary.has_value()) {
                throw std::runtime_error(
                    "Sequence " +
                    std::to_string(resource.id) +
                    " references missing secondary frame " +
                    std::to_string(
                        *frame.secondaryFrameId
                    )
                );
            }

            secondary.emplace(
                ResolvedAnimationFrame{
                    &resolvedSecondary->frame,
                    resolvedSecondary->skeleton
                }
            );
        }

        resource.resolvedFrames.push_back({
            ResolvedAnimationFrame{
                &primary->frame,
                primary->skeleton
            },
            secondary
        });
    }

    return resource;
}

}
