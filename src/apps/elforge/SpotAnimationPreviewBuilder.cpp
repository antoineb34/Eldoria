#include "SpotAnimationPreviewBuilder.h"

#include <cstddef>

namespace eld::elforge {

namespace {

void rotate90Degrees(
    eld::model::ModelMesh& mesh
) {
    for (eld::model::Vertex& vertex : mesh.vertices) {
        const float x = vertex.x;
        vertex.x = vertex.z;
        vertex.z = -x;
    }
}

}

std::optional<eld::model::Model>
SpotAnimationPreviewBuilder::buildAnimationSource(
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
    return model;
}

void SpotAnimationPreviewBuilder::prepareAnimatedMesh(
    const eld::definition::SpotAnimationDefinition& definition,
    eld::model::ModelMesh& mesh
) const {
    for (eld::model::Vertex& vertex : mesh.vertices) {
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

    for (eld::model::Face& face : mesh.faces) {
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

    if (definition.rotation == 90) {
        rotate90Degrees(mesh);
    }
    else if (definition.rotation == 180) {
        rotate90Degrees(mesh);
        rotate90Degrees(mesh);
    }
    else if (definition.rotation == 270) {
        rotate90Degrees(mesh);
        rotate90Degrees(mesh);
        rotate90Degrees(mesh);
    }
}

std::optional<eld::model::Model>
SpotAnimationPreviewBuilder::build(
    const eld::definition::SpotAnimationDefinition& definition,
    const eld::model::ModelRepository& repository
) const {
    std::optional<eld::model::Model> model =
        buildAnimationSource(
            definition,
            repository
        );

    if (!model.has_value()) {
        return std::nullopt;
    }

    prepareAnimatedMesh(
        definition,
        model->mesh
    );

    return model;
}

}
