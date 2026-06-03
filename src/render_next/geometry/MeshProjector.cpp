#include "MeshProjector.h"

#include <cmath>

#include "../scene/Transform.h"

#include "../../render/software/camera/Projection.h"
#include "../../render/software/math/Mat4.h"

namespace rf::render_next {

namespace {

rf::render::Vec3 toLocalVec3(
    const rf::model::Vertex& vertex
) {
    return {
        static_cast<float>(vertex.x),
        -static_cast<float>(vertex.y),
        static_cast<float>(vertex.z)
    };
}

bool isFiniteScreenPoint(
    const rf::render::ScreenPoint& point
) {
    return
        std::isfinite(point.x) &&
        std::isfinite(point.y) &&
        std::isfinite(point.z);
}

rf::render::ScreenPoint toScreenPoint(
    const rf::render::Vec3& projected,
    const rf::render::Vec3& viewPoint,
    const RenderCamera& camera
) {
    rf::render::ScreenPoint screen {};

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

    const rf::render::Mat4 modelMatrix =
        buildModelMatrix(object.transform);

    const rf::render::Mat4 viewMatrix =
        rf::render::buildViewMatrix(camera);

    const rf::render::Mat4 projectionMatrix =
        rf::render::buildProjectionMatrix(camera);

    mesh.vertices.reserve(object.model->vertices.size());

    for (const rf::model::Vertex& sourceVertex : object.model->vertices) {
        const rf::render::Vec3 local =
            toLocalVec3(sourceVertex);

        const rf::render::Vec3 world =
            modelMatrix.transformPoint(local);

        const rf::render::Vec3 view =
            viewMatrix.transformPoint(world);

        const rf::render::Vec3 projected =
            projectionMatrix.transformPoint(view);

        const rf::render::ScreenPoint screen =
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
