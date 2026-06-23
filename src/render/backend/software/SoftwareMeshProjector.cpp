#include "SoftwareMeshProjector.h"

#include <cmath>

#include "math/Mat4.h"

namespace eld::render {

namespace {

bool isVisible(
    const ScreenPoint& point,
    const Camera& camera
) {
    return
        std::isfinite(point.x) &&
        std::isfinite(point.y) &&
        std::isfinite(point.depth) &&
        point.depth >= camera.nearPlane &&
        point.depth <= camera.farPlane;
}

}

SoftwareProjectedMesh SoftwareMeshProjector::project(
    const eld::graphics::RenderMesh& mesh,
    const Transform& transform,
    const Camera& camera
) const {
    SoftwareProjectedMesh result;

    const eld::math::Mat4 modelMatrix =
        buildModelMatrix(
            transform
        );

    const eld::math::Mat4 viewMatrix =
        buildViewMatrix(
            camera
        );

    const eld::math::Mat4 projectionMatrix =
        buildProjectionMatrix(
            camera
        );

    result.vertices.reserve(
        mesh.vertices.size()
    );

    for (
        const eld::graphics::RenderVertex& vertex :
        mesh.vertices
    ) {
        const eld::math::Vec3 worldPosition =
            modelMatrix.transformPoint(
                vertex.position
            );

        const ScreenPoint screen =
            projectPoint(
                worldPosition,
                viewMatrix,
                projectionMatrix,
                camera
            );

        result.vertices.push_back({
            screen,
            vertex.uv,
            vertex.color,
            isVisible(screen, camera)
        });
    }

    return result;
}

}
