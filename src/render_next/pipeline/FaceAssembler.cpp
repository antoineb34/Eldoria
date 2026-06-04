#include "FaceAssembler.h"

#include <algorithm>

namespace rf::render_next {

namespace {

bool isValidFace(
    const rf::model::Face& face,
    const ProjectedMesh& mesh
) {
    return
        face.a >= 0 &&
        face.b >= 0 &&
        face.c >= 0 &&
        face.a < static_cast<int>(mesh.vertices.size()) &&
        face.b < static_cast<int>(mesh.vertices.size()) &&
        face.c < static_cast<int>(mesh.vertices.size());
}

bool hasValidProjectedVertices(
    const rf::model::Face& face,
    const ProjectedMesh& mesh
) {
    return
        mesh.vertices[face.a].valid &&
        mesh.vertices[face.b].valid &&
        mesh.vertices[face.c].valid;
}

RenderPacket makePacket(
    int objectIndex,
    int faceIndex,
    const rf::model::Face& face,
    const ProjectedMesh& mesh
) {
    const float z0 = mesh.vertices[face.a].screen.z;
    const float z1 = mesh.vertices[face.b].screen.z;
    const float z2 = mesh.vertices[face.c].screen.z;

    return RenderPacket {
        objectIndex,
        faceIndex,

        face.a,
        face.b,
        face.c,

        (z0 + z1 + z2) / 3.0f,
        std::min({ z0, z1, z2 }),
        std::max({ z0, z1, z2 }),

        face.color,
        face.alpha,
        face.renderType,
        face.priority,

        face.texturePointer,
        face.textureUVMappingIndex,

        true
    };
}

}

RenderQueue FaceAssembler::assemble(
    int objectIndex,
    const RenderObject& object,
    const ProjectedMesh& mesh
) const {
    RenderQueue queue;

    if (object.model == nullptr) {
        return queue;
    }

    queue.packets.reserve(object.model->faces.size());

    for (int faceIndex = 0;
         faceIndex < static_cast<int>(object.model->faces.size());
         faceIndex++) {
        const rf::model::Face& face =
            object.model->faces[faceIndex];

        if (!isValidFace(face, mesh)) {
            continue;
        }

        if (!hasValidProjectedVertices(face, mesh)) {
            continue;
        }

        queue.packets.push_back(
            makePacket(
                objectIndex,
                faceIndex,
                face,
                mesh
            )
        );
    }

    return queue;
}

}
