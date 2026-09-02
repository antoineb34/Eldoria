#include "views/npc/NpcView.h"

#include <cstdint>
#include <utility>

namespace eld::elforge {

namespace {

void prepareMesh(
    eld::model::Model& mesh,
    const eld::definition::NpcDefinition& definition
) {
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
            const eld::definition::NpcRecolor& recolor :
            definition.recolors
        ) {
            if (face.color == recolor.source) {
                face.color = recolor.destination;
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

}

std::optional<eld::model::Model>
NpcView::build(
    const eld::definition::NpcDefinition& definition,
    const eld::model::ModelRepository& repository
) const {
    eld::model::Model combined;
    combined.id = definition.id;

    bool found = false;

    for (const std::uint16_t modelId : definition.modelIds) {
        std::optional<eld::model::Model> model =
            repository.find(modelId);

        if (!model.has_value()) {
            continue;
        }

        prepareMesh(
            *model,
            definition
        );

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

}
