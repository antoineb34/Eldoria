#include "ModelSkinGroups.h"

namespace eld::render {

ModelSkinGroups ModelSkinGroups::build(
    const eld::model::ModelMesh& mesh
) {
    ModelSkinGroups groups;

    for (
        std::size_t vertexIndex = 0;
        vertexIndex < mesh.vertices.size();
        ++vertexIndex
    ) {
        const eld::model::Vertex& vertex =
            mesh.vertices[vertexIndex];

        if (!vertex.skin.has_value()) {
            continue;
        }

        groups.verticesBySkin_[
            *vertex.skin
        ].push_back(vertexIndex);
    }

    for (
        std::size_t faceIndex = 0;
        faceIndex < mesh.faces.size();
        ++faceIndex
    ) {
        const eld::model::Face& face =
            mesh.faces[faceIndex];

        if (!face.skin.has_value()) {
            continue;
        }

        groups.facesBySkin_[
            *face.skin
        ].push_back(faceIndex);
    }

    return groups;
}

const std::vector<std::size_t>&
ModelSkinGroups::vertices(
    std::uint8_t groupId
) const {
    return verticesBySkin_[groupId];
}

const std::vector<std::size_t>&
ModelSkinGroups::faces(
    std::uint8_t groupId
) const {
    return facesBySkin_[groupId];
}

}
