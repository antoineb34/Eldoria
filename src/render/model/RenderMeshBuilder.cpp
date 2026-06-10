#include "RenderMeshBuilder.h"

#include <algorithm>
#include <cmath>

#include "../software/math/Mat4.h"
#include "../software/camera/Projection.h"

namespace eld::render {

namespace {

bool isValidFace(
    const eld::model::Face& face,
    const std::vector<eld::model::Vertex>& vertices
) {
    return
        face.a >= 0 &&
        face.b >= 0 &&
        face.c >= 0 &&
        face.a < static_cast<int>(vertices.size()) &&
        face.b < static_cast<int>(vertices.size()) &&
        face.c < static_cast<int>(vertices.size());
}

bool isFinitePoint(const ScreenPoint& point) {
    return
        std::isfinite(point.x) &&
        std::isfinite(point.y) &&
        std::isfinite(point.z);
}

float screenArea(
    const ScreenPoint& a,
    const ScreenPoint& b,
    const ScreenPoint& c
) {
    return
        (b.x - a.x) * (c.y - a.y) -
        (b.y - a.y) * (c.x - a.x);
}

eld::model::Vertex transformVertex(
    eld::model::Vertex vertex,
    const ModelTransform& transform
) {
    float x = vertex.x * transform.scale;
    float y = vertex.y * transform.scale;
    float z = vertex.z * transform.scale;

    float cosX = std::cos(transform.rotationX);
    float sinX = std::sin(transform.rotationX);

    float y1 = y * cosX - z * sinX;
    float z1 = y * sinX + z * cosX;

    y = y1;
    z = z1;

    float cosY = std::cos(transform.rotationY);
    float sinY = std::sin(transform.rotationY);

    float x1 = x * cosY + z * sinY;
    float z2 = -x * sinY + z * cosY;

    x = x1;
    z = z2;

    float cosZ = std::cos(transform.rotationZ);
    float sinZ = std::sin(transform.rotationZ);

    float x2 = x * cosZ - y * sinZ;
    float y2 = x * sinZ + y * cosZ;

    return {
        x2 + transform.offsetX,
        y2 + transform.offsetY,
        z + transform.offsetZ
    };
}

ScreenPoint projectTransformedVertex(
    const eld::model::Vertex& vertex,
    const ModelTransform& transform,
    const Mat4& view,
    const Mat4& projection,
    const Camera& camera
) {
    eld::model::Vertex transformed =
        transformVertex(vertex, transform);

    return projectVertex(
        transformed,
        view,
        projection,
        camera
    );
}

}

RenderMesh RenderMeshBuilder::build(
    const eld::model::ModelAsset& model,
    const Camera& camera,
    const ModelTransform& transform
) {
    Mat4 view = buildViewMatrix(camera);
    Mat4 projection = buildProjectionMatrix(camera);

    RenderMesh mesh;
    mesh.vertices.reserve(model.vertices.size());
    mesh.faces.reserve(model.faces.size());

    for (const eld::model::Vertex& vertex : model.vertices) {
        ScreenPoint screen =
            projectTransformedVertex(
                vertex,
                transform,
                view,
                projection,
                camera
            );

        mesh.vertices.push_back({
            screen
        });
    }

    for (int faceIndex = 0; faceIndex < static_cast<int>(model.faces.size()); faceIndex++) {
        const eld::model::Face& face = model.faces[faceIndex];

        if (!isValidFace(face, model.vertices)) {
            continue;
        }

        const ScreenPoint& a = mesh.vertices[face.a].screen;
        const ScreenPoint& b = mesh.vertices[face.b].screen;
        const ScreenPoint& c = mesh.vertices[face.c].screen;

        if (
            !isFinitePoint(a) ||
            !isFinitePoint(b) ||
            !isFinitePoint(c)
        ) {
            continue;
        }

        if (screenArea(a, b, c) <= 0.0f) {
            continue;
        }

        float z0 = a.z;
        float z1 = b.z;
        float z2 = c.z;

        mesh.faces.push_back({
            faceIndex,
            &face,
            face.a,
            face.b,
            face.c,
            (z0 + z1 + z2) / 3.0f,
            std::min({ z0, z1, z2 }),
            std::max({ z0, z1, z2 })
        });
    }

    return mesh;
}

}
