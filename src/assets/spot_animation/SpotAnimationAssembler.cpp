#include "spot_animation/SpotAnimationAssembler.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace eld::spot_animation {

SpotAnimationAssembler::SpotAnimationAssembler(
    const eld::model::ModelLoader& models,
    const eld::sequence::SequenceLoader& sequences
)
    : models_(&models),
      sequences_(&sequences) {
}


SpotAnimationResource SpotAnimationAssembler::assemble(
    SpotAnimationData data
) const {
    if (models_ == nullptr || sequences_ == nullptr) {
        throw std::logic_error(
            "Spot animation assembler requires model and sequence loaders"
        );
    }

    SpotAnimationResource resource{
        .data = std::move(data),
        .model = nullptr,
        .sequence = nullptr
    };

    if (resource.data.modelId.has_value()) {
        if (!models_->contains(*resource.data.modelId)) {
            throw std::runtime_error(
                "Spot animation " + std::to_string(resource.data.id) +
                " references missing model " +
                std::to_string(*resource.data.modelId)
            );
        }

        resource.model = &models_->resource(*resource.data.modelId);
    }

    if (resource.data.sequenceId.has_value()) {
        if (!sequences_->contains(*resource.data.sequenceId)) {
            throw std::runtime_error(
                "Spot animation " + std::to_string(resource.data.id) +
                " references missing sequence " +
                std::to_string(*resource.data.sequenceId)
            );
        }

        resource.sequence = &sequences_->resource(*resource.data.sequenceId);
    }

    return resource;
}

}
