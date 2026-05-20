#include "render/Mesh.h"

Mesh Mesh::fromModel(const Model& model) {
    Mesh mesh;

    for (size_t i = 0; i < model.vertexX.size(); i++) {
        Vertex v;
        v.x = static_cast<float>(model.vertexX[i]);
        v.y = static_cast<float>(model.vertexY[i]);
        v.z = static_cast<float>(model.vertexZ[i]);
        mesh.vertices.push_back(v);
    }

    for (size_t i = 0; i < model.triA.size(); i++) {
        mesh.indices.push_back(model.triA[i]);
        mesh.indices.push_back(model.triB[i]);
        mesh.indices.push_back(model.triC[i]);
    }

    return mesh;
}
