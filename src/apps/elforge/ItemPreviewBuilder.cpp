#include "ItemPreviewBuilder.h"

#include <utility>

namespace eld::elforge {

std::optional<eld::model::Model>
ItemPreviewBuilder::build(
    const eld::definition::ItemDefinition& definition,
    const eld::model::ModelRepository& repository
) const {
    if (!definition.inventoryModelId.has_value()) {
        return std::nullopt;
    }

    std::optional<eld::model::Model> model =
        repository.find(
            *definition.inventoryModelId
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
            static_cast<float>(definition.scaleZ) /
            128.0f;
    }

    for (eld::model::Face& face : model->mesh.faces) {
        for (
            const eld::definition::ItemRecolor& recolor :
            definition.recolors
        ) {
            if (face.color == recolor.source) {
                face.color = recolor.destination;
                break;
            }
        }
    }

    return model;
}

}
