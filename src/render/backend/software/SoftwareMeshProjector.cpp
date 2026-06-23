#include "SoftwareMeshProjector.h"

#include <cmath>

#include "../../math/Mat4.h"
#include "../../scene/Transform.h"

namespace eld::render {

namespace {

bool isFinite(
    const ScreenPoint& point
) {
    return
        std::isfinite(point.x) &&
        std::isfinite(point.y) &&
        std::isfinite(point.z);
}

ScreenPoint toScreenPoint(
    const Vec3& projected,
    const Vec3& view,
    const RenderCamera& camera
) {
    return ScreenPoint{
        .x =
            (projected.x + 1.0f) *
            0.5f *
            static_cast<float>(
                camera.viewportWidth
            ),
        .y =
            (1.0f - projected.y) *
            0.5f *
            static_cast<float>(
                camera.viewportHeight
            ),
        .z = view.z
    };
}

}

SoftwareProjectedMesh
SoftwareMeshProjector::project(
    const RenderObject& object,
    const RenderCamera& camera
) const {
    SoftwareProjectedMesh projectedMesh;

    if (object.model == nullptr) {
        return projectedMesh;
    }

    const Mat4 modelMatrix =
        buildModelMatrix(
            object.transform
        );

    const Mat4 viewMatrix =
        buildViewMatrix(
            camera
        );

    const Mat4 projectionMatrix =
        buildProjectionMatrix(
            camera
        );

    const RenderMesh& mesh =
        object.model->mesh;

    projectedMesh.vertices.reserve(
        mesh.vertices.size()
    );

    for (
        const RenderVertex& vertex :
        mesh.vertices
    ) {
        const Vec3 world =
            modelMatrix.transformPoint(
                vertex.position
            );

        const Vec3 view =
            viewMatrix.transformPoint(
                world
            );

        const Vec3 projected =
            projectionMatrix.transformPoint(
                view
            );

        const ScreenPoint screen =
            toScreenPoint(
                projected,
                view,
                camera
            );

        projectedMesh.vertices.push_back(
            SoftwareProjectedVertex{
                .world = world,
                .view = view,
                .screen = screen,
                .uv = vertex.uv,
                .color = vertex.color,
                .valid = isFinite(screen)
            }
        );
    }

    return projectedMesh;
}

}
