#pragma once

#include "../model/VertexDecoder.h"

namespace rf::render {

struct Camera {

    float centerX = 0.0f;
    float centerY = 0.0f;

    float angleX = 0.0f;
    float angleY = 0.0f;

    float scale = 1.0f;
};

struct ScreenPoint {

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

ScreenPoint projectVertex(
    const rf::model::Vertex& vertex,
    const Camera& camera
);

}
