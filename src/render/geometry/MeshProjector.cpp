#include "MeshProjector.h"

#include <cmath>

#include "../scene/Transform.h"

#include "../../render/camera/Projection.h"
#include "../../render/math/Mat4.h"

namespace eld::render {

namespace {

eld::render::Vec3 toLocalVec3(
    const eld::model::Vertex& vertex
) {
    return {
        static_cast<float>(vertex.x),
        -static_cast<float>(vertex.y),
        static_cast<float>(vertex.z)
    };
}

bool isFiniteScreenPoint(
    const eld::render::ScreenPoint& point
) {
    return
        std::isfinite(point.x) &&
        std::isfinite(point.y) &&
        std::isfinite(point.z);
}

eld::render::ScreenPoint toScreenPoint(
    const eld::render::Vec3& projected,
    const eld::render::Vec3& viewPoint,
    const RenderCamera& camera
) {
    eld::render::ScreenPoint screen {};

    screen.x =
        (projected.x + 1.0f) * 0.5f *
        static_cast<float>(camera.viewportWidth);

    screen.y =
        (1.0f - projected.y) * 0.5f *
        static_cast<float>(camera.viewportHeight);

    screen.z = viewPoint.z;

    return screen;
}

}

ProjectedMesh MeshProjector::project(
    const RenderObject& object,
    const RenderCamera& camera
) const {
    ProjectedMesh mesh;

    if (object.model == nullptr) {
        return mesh;
    }

    const eld::render::Mat4 modelMatrix =
        buildModelMatrix(object.transform);

    const eld::render::Mat4 viewMatrix =
        eld::render::buildViewMatrix(camera);

    const eld::render::Mat4 projectionMatrix =
        eld::render::buildProjectionMatrix(camera);

    mesh.vertices.reserve(object.model->vertices.size());

    for (const eld::model::Vertex& sourceVertex : object.model->vertices) {
        const eld::render::Vec3 local =
            toLocalVec3(sourceVertex);

        const eld::render::Vec3 world =
            modelMatrix.transformPoint(local);

        const eld::render::Vec3 view =
            viewMatrix.transformPoint(world);

        const eld::render::Vec3 projected =
            projectionMatrix.transformPoint(view);

        const eld::render::ScreenPoint screen =
            toScreenPoint(
                projected,
                view,
                camera
            );

        mesh.vertices.push_back(ProjectedVertex {
            local,
            world,
            view,
            screen,
            isFiniteScreenPoint(screen)
        });
    }

    return mesh;
}

}
