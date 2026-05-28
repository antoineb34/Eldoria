#include "Projection.h"

namespace rf::render {

Vec3 toVec3(
    const rf::model::Vertex& vertex
) {
    return {
        static_cast<float>(vertex.x),
        -static_cast<float>(vertex.y),
        static_cast<float>(vertex.z)
    };
}

Mat4 buildViewMatrix(
    const Camera& camera
) {
    Mat4 rotationX =
        Mat4::rotationX(
            camera.angleX
        );

    Mat4 rotationY =
        Mat4::rotationY(
            camera.angleY
        );

    Mat4 translation =
        Mat4::translation({
            0.0f,
            0.0f,
            camera.distance
        });

    return
        rotationX *
        rotationY *
        translation;
}

Mat4 buildProjectionMatrix(
    const Camera& camera
) {
    float aspectRatio =
        static_cast<float>(
            camera.viewportWidth
        ) /
        static_cast<float>(
            camera.viewportHeight
        );

    return Mat4::perspective(
        camera.fov,
        aspectRatio,
        camera.nearPlane,
        camera.farPlane
    );
}

ScreenPoint projectPoint(
    const Vec3& point,
    const Mat4& transform,
    const Camera& camera
) {
    Vec3 projected =
        transform.transformPoint(
            point
        );

    ScreenPoint screenPoint {};

    screenPoint.x =
        static_cast<float>(camera.viewportX) +
        (projected.x + 1.0f) *
        0.5f *
        static_cast<float>(
            camera.viewportWidth
        );

    screenPoint.y =
        static_cast<float>(camera.viewportY) +
        (1.0f - projected.y) *
        0.5f *
        static_cast<float>(
            camera.viewportHeight
        );

    screenPoint.z =
        projected.z;

    return screenPoint;
}

ScreenPoint projectVertex(
    const rf::model::Vertex& vertex,
    const Mat4& transform,
    const Camera& camera
) {
    return projectPoint(
        toVec3(vertex),
        transform,
        camera
    );
}

}
