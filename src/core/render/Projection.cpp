#include "Projection.h"

#include <cmath>

namespace rf::render {

ScreenPoint projectVertex(
    const rf::model::Vertex& vertex,
    const Camera& camera
) {

    float cosY =
        std::cos(camera.angleY);

    float sinY =
        std::sin(camera.angleY);

    float cosX =
        std::cos(camera.angleX);

    float sinX =
        std::sin(camera.angleX);

    float x =
        static_cast<float>(vertex.x);

    float y =
        -static_cast<float>(vertex.y);

    float z =
        static_cast<float>(vertex.z);

    float x1 =
        x * cosY +
        z * sinY;

    float z1 =
        -x * sinY +
        z * cosY;

    float y2 =
        y * cosX -
        z1 * sinX;

    float z2 =
        y * sinX +
        z1 * cosX;

    ScreenPoint point {};

    point.x =
        camera.centerX +
        x1 * camera.scale;

    point.y =
        camera.centerY -
        y2 * camera.scale;

    point.z =
        z2;

    return point;
}

}
