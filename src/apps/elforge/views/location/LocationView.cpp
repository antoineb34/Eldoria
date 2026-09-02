#include "views/location/LocationView.h"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace eld::elforge {

namespace {

void prepareMesh(
    eld::model::Model& mesh,
    const eld::definition::LocationDefinition& definition
) {
    for (eld::model::Vertex& vertex : mesh.vertices) {
        vertex.x =
            vertex.x *
            static_cast<float>(definition.scaleX) /
            128.0f +
            static_cast<float>(definition.offsetX);

        vertex.y =
            vertex.y *
            static_cast<float>(definition.scaleY) /
            128.0f +
            static_cast<float>(definition.offsetY);

        vertex.z =
            vertex.z *
            static_cast<float>(definition.scaleZ) /
            128.0f +
            static_cast<float>(definition.offsetZ);
    }

    for (eld::model::Face& face : mesh.faces) {
        for (
            const eld::definition::LocationRecolor& recolor :
            definition.recolors
        ) {
            if (face.color == recolor.source) {
                face.color =
                    recolor.destination;
                break;
            }
        }
    }
}

void appendMesh(
    eld::model::Model& destination,
    eld::model::Model source
) {
    const std::uint32_t vertexOffset =
        static_cast<std::uint32_t>(
            destination.vertices.size()
        );

    const std::uint32_t mappingOffset =
        static_cast<std::uint32_t>(
            destination.textureMappings.size()
        );

    for (eld::model::Face& face : source.faces) {
        face.a += vertexOffset;
        face.b += vertexOffset;
        face.c += vertexOffset;

        if (face.textureMappingIndex.has_value()) {
            *face.textureMappingIndex +=
                mappingOffset;
        }
    }

    for (
        eld::model::TextureMapping& mapping :
        source.textureMappings
    ) {
        mapping.originVertex += vertexOffset;
        mapping.uVertex += vertexOffset;
        mapping.vVertex += vertexOffset;
    }

    destination.vertices.insert(
        destination.vertices.end(),
        source.vertices.begin(),
        source.vertices.end()
    );

    destination.faces.insert(
        destination.faces.end(),
        source.faces.begin(),
        source.faces.end()
    );

    destination.textureMappings.insert(
        destination.textureMappings.end(),
        source.textureMappings.begin(),
        source.textureMappings.end()
    );
}

std::vector<std::uint16_t> selectModels(
    const eld::definition::LocationDefinition& definition
) {
    std::vector<std::uint16_t> ids;

    bool hasTypedModels = false;

    for (
        const eld::definition::LocationModel& model :
        definition.models
    ) {
        if (model.type.has_value()) {
            hasTypedModels = true;
            break;
        }
    }

    if (!hasTypedModels) {
        for (
            const eld::definition::LocationModel& model :
            definition.models
        ) {
            ids.push_back(model.id);
        }

        return ids;
    }

    for (
        const eld::definition::LocationModel& model :
        definition.models
    ) {
        if (
            model.type.has_value() &&
            *model.type == 10
        ) {
            ids.push_back(model.id);
            return ids;
        }
    }

    if (!definition.models.empty()) {
        ids.push_back(
            definition.models.front().id
        );
    }

    return ids;
}

}

std::optional<eld::model::Model>
LocationView::buildAnimationSource(
    const eld::definition::LocationDefinition& definition,
    const eld::model::ModelRepository& repository
) const {
    eld::model::Model combined;
    combined.id = definition.id;

    bool found = false;

    for (
        const std::uint16_t modelId :
        selectModels(definition)
    ) {
        std::optional<eld::model::Model> model =
            repository.find(modelId);

        if (!model.has_value()) {
            continue;
        }

        appendMesh(
            combined,
            std::move(*model)
        );

        found = true;
    }

    return found
        ? std::optional{std::move(combined)}
        : std::nullopt;
}

void LocationView::prepareAnimatedMesh(
    const eld::definition::LocationDefinition& definition,
    eld::model::Model& mesh
) const {
    prepareMesh(
        mesh,
        definition
    );
}

std::optional<eld::model::Model>
LocationView::build(
    const eld::definition::LocationDefinition& definition,
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
        *model
    );

    return model;
}

}
