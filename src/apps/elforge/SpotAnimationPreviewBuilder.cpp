#include "SpotAnimationPreviewBuilder.h"

namespace eld::elforge {

std::optional<eld::model::Model>
SpotAnimationPreviewBuilder::build(
    const eld::definition::SpotAnimationDefinition& definition,
    const eld::model::ModelRepository& repository
) const {
    if (!definition.modelId.has_value()) {
        return std::nullopt;
    }

    std::optional<eld::model::Model> model =
        repository.find(
            *definition.modelId
        );

    if (!model.has_value()) {
        return std::nullopt;
    }

    model->id = definition.id;

    for (eld::model::Vertex& vertex : model->mesh.vertices) {
        vertex.x =
            vertex.x *
            static_cast<float>(definition.scaleX) /
            128.0f;

        vertex.y =
            vertex.y *
            static_cast<float>(definition.scaleY) /
            128.0f;

        vertex.z =
            vertex.z *
            static_cast<float>(definition.scaleX) /
            128.0f;
    }

    for (eld::model::Face& face : model->mesh.faces) {
        for (
            std::size_t index = 0;
            index < definition.recolorSources.size();
            ++index
        ) {
            if (
                definition.recolorSources[index].has_value() &&
                definition.recolorDestinations[index].has_value() &&
                face.color ==
                    *definition.recolorSources[index]
            ) {
                face.color =
                    *definition.recolorDestinations[index];

                break;
            }
        }
    }

    return model;
}

}
