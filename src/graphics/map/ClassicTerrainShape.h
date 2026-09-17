#pragma once

#include <vector>

#include "world/Position.h"

namespace eld::graphics::terrain {

enum class HeightRule {
    Surface,

    SouthEdge,
    EastEdge,
    NorthEdge,
    WestEdge,

    Southwest,
    Southeast,
    Northeast,
    Northwest
};


struct ClassicPoint {
    eld::world::TileLocalPosition local{};
    HeightRule heightRule =
        HeightRule::Surface;
};


const ClassicPoint& point(
    int type
);


const std::vector<int>& pointPattern(
    int shape
);


const std::vector<int>& elementPattern(
    int shape
);


int rotatePointType(
    int type,
    int rotation
);


int rotateTriangleIndex(
    int index,
    int rotation
);

}
