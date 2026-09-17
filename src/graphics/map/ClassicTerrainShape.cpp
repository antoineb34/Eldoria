#include "ClassicTerrainShape.h"

#include <array>
#include <stdexcept>

namespace eld::graphics::terrain {

namespace {

constexpr std::array<
    ClassicPoint,
    17
> Points{{
    // 0 unused
    {{0.00f, 0.00f}, HeightRule::Surface},

    // 1-8: corners and edge midpoints
    {{0.00f, 0.00f}, HeightRule::Surface},
    {{0.50f, 0.00f}, HeightRule::Surface},
    {{1.00f, 0.00f}, HeightRule::Surface},
    {{1.00f, 0.50f}, HeightRule::Surface},
    {{1.00f, 1.00f}, HeightRule::Surface},
    {{0.50f, 1.00f}, HeightRule::Surface},
    {{0.00f, 1.00f}, HeightRule::Surface},
    {{0.00f, 0.50f}, HeightRule::Surface},

    // 9-12: interior positions using classic edge heights
    {{0.50f, 0.25f}, HeightRule::SouthEdge},
    {{0.75f, 0.50f}, HeightRule::EastEdge},
    {{0.50f, 0.75f}, HeightRule::NorthEdge},
    {{0.25f, 0.50f}, HeightRule::WestEdge},

    // 13-16: interior positions using corner heights
    {{0.25f, 0.25f}, HeightRule::Southwest},
    {{0.75f, 0.25f}, HeightRule::Southeast},
    {{0.75f, 0.75f}, HeightRule::Northeast},
    {{0.25f, 0.75f}, HeightRule::Northwest}
}};


const std::vector<
    std::vector<int>
> PointPatterns{
    {1,3,5,7},
    {1,3,5,7},
    {1,3,5,7},
    {1,3,5,7,6},
    {1,3,5,7,6},
    {1,3,5,7,6},
    {1,3,5,7,6},
    {1,3,5,7,2,6},
    {1,3,5,7,2,8},
    {1,3,5,7,2,8},
    {1,3,5,7,11,12},
    {1,3,5,7,11,12},
    {1,3,5,7,13,14}
};


const std::vector<
    std::vector<int>
> ElementPatterns{
    {0,1,2,3, 0,0,1,3},

    {1,1,2,3, 1,0,1,3},

    {0,1,2,3, 1,0,1,3},

    {0,0,1,2,
     0,0,2,4,
     1,0,4,3},

    {0,0,1,4,
     0,0,4,3,
     1,1,2,4},

    {0,0,4,3,
     1,0,1,2,
     1,0,2,4},

    {0,1,2,4,
     1,0,1,4,
     1,0,4,3},

    {0,4,1,2,
     0,4,2,5,
     1,0,4,5,
     1,0,5,3},

    {0,4,1,2,
     0,4,2,3,
     0,4,3,5,
     1,0,4,5},

    {0,0,4,5,
     1,4,1,2,
     1,4,2,3,
     1,4,3,5},

    {0,0,1,5,
     0,1,4,5,
     0,1,2,4,
     1,0,5,3,
     1,5,4,3,
     1,4,2,3},

    {1,0,1,5,
     1,1,4,5,
     1,1,2,4,
     0,0,5,3,
     0,5,4,3,
     0,4,2,3},

    {1,0,5,4,
     1,0,1,5,
     0,0,4,3,
     0,4,5,3,
     0,5,2,3,
     0,1,2,5}
};

}


const ClassicPoint& point(
    int type
) {
    if (
        type <= 0 ||
        type >= static_cast<int>(
            Points.size()
        )
    ) {
        throw std::out_of_range(
            "Invalid classic terrain point type"
        );
    }

    return Points[
        static_cast<std::size_t>(type)
    ];
}


const std::vector<int>& pointPattern(
    int shape
) {
    return PointPatterns.at(
        static_cast<std::size_t>(shape)
    );
}


const std::vector<int>& elementPattern(
    int shape
) {
    return ElementPatterns.at(
        static_cast<std::size_t>(shape)
    );
}


int rotatePointType(
    int type,
    int rotation
) {
    if (
        (type & 1) == 0 &&
        type <= 8
    ) {
        return
            (
                (
                    type -
                    rotation * 2 -
                    1
                ) & 7
            ) + 1;
    }

    if (
        type > 8 &&
        type <= 12
    ) {
        return
            (
                (
                    type -
                    9 -
                    rotation
                ) & 3
            ) + 9;
    }

    if (
        type > 12 &&
        type <= 16
    ) {
        return
            (
                (
                    type -
                    13 -
                    rotation
                ) & 3
            ) + 13;
    }

    return type;
}


int rotateTriangleIndex(
    int index,
    int rotation
) {
    if (index < 4) {
        return
            (
                index -
                rotation
            ) & 3;
    }

    return index;
}

}
