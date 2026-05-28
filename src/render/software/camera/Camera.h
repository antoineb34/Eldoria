#pragma once

namespace rf::render {

struct Camera {
    float angleX = 0.0f;
    float angleY = 0.0f;

    float distance = 500.0f;

    float fov = 1.04719755f;

    float nearPlane = 1.0f;
    float farPlane = 10000.0f;

    int viewportX = 0;
    int viewportY = 0;

    int viewportWidth = 800;
    int viewportHeight = 600;
};

}
