#include "sequence/SequenceAssembler.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace eld::sequence {

SequenceAssembler::SequenceAssembler(
    const eld::animation::AnimationFrameTable& frames
)
    : frames_(&frames) {
}


SequenceResource SequenceAssembler::assemble(
    SequenceData data
) const {
    if (frames_ == nullptr) {
        throw std::logic_error(
            "Sequence assembler requires an animation frame table"
        );
    }

    SequenceResource resource{
        .data = std::move(data),
        .resolvedFrames = {}
    };

    resource.resolvedFrames.reserve(
        resource.data.frames.size()
    );

    for (
        const SequenceFrameData& frame :
        resource.data.frames
    ) {
        const auto primary =
            frames_->find(
                frame.primaryFrameId
            );

        if (!primary.has_value()) {
            throw std::runtime_error(
                "Sequence " +
                std::to_string(resource.data.id) +
                " references missing primary frame " +
                std::to_string(frame.primaryFrameId)
            );
        }

        std::optional<AnimationFrameHandle> secondary;

        if (frame.secondaryFrameId.has_value()) {
            const auto resolvedSecondary =
                frames_->find(
                    *frame.secondaryFrameId
                );

            if (!resolvedSecondary.has_value()) {
                throw std::runtime_error(
                    "Sequence " +
                    std::to_string(resource.data.id) +
                    " references missing secondary frame " +
                    std::to_string(
                        *frame.secondaryFrameId
                    )
                );
            }

            secondary.emplace(
                AnimationFrameHandle{
                    &resolvedSecondary->frame,
                    resolvedSecondary->skeleton
                }
            );
        }

        resource.resolvedFrames.push_back({
            AnimationFrameHandle{
                &primary->frame,
                primary->skeleton
            },
            secondary
        });
    }

    return resource;
}

}
