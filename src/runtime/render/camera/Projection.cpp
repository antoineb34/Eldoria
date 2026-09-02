#include "Projection.h"

namespace eld::render {

eld::math::Mat4 buildViewMatrix(
    const Camera& camera
) {
    const eld::math::Mat4 translation =
        eld::math::Mat4::translation({
            -camera.position.x,
            -camera.position.y,
            -camera.position.z
        });

    const eld::math::Mat4 rotationZ =
        eld::math::Mat4::rotationZ(
            -camera.rotation.z
        );

    const eld::math::Mat4 rotationY =
        eld::math::Mat4::rotationY(
            -camera.rotation.y
        );

    const eld::math::Mat4 rotationX =
        eld::math::Mat4::rotationX(
            -camera.rotation.x
        );

    return
        translation *
        rotationZ *
        rotationY *
        rotationX;
}

eld::math::Mat4 buildProjectionMatrix(
    const Camera& camera
) {
    const float aspectRatio =
        camera.viewportHeight == 0
            ? 1.0f
            : static_cast<float>(
                  camera.viewportWidth
              ) /
              static_cast<float>(
                  camera.viewportHeight
              );

    return eld::math::Mat4::perspective(
        camera.verticalFov,
        aspectRatio,
        camera.nearPlane,
        camera.farPlane
    );
}

ScreenPoint projectPoint(
    const eld::math::Vec3& worldPoint,
    const eld::math::Mat4& viewMatrix,
    const eld::math::Mat4& projectionMatrix,
    const Camera& camera
) {
    const eld::math::Vec3 viewPoint =
        viewMatrix.transformPoint(
            worldPoint
        );

    const eld::math::Vec3 projected =
        projectionMatrix.transformPoint(
            viewPoint
        );

    return {
        (projected.x + 1.0f) *
            0.5f *
            static_cast<float>(
                camera.viewportWidth
            ),

        (1.0f - projected.y) *
            0.5f *
            static_cast<float>(
                camera.viewportHeight
            ),

        viewPoint.z
    };
}

}
