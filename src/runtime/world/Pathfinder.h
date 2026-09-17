#pragma once

#include <vector>

#include "Position.h"
#include "World.h"


namespace eld::world
{

    enum class PathStatus
    {
        Success,
        SameTile,
        PlaneMismatch,
        RegionMissing,
        CrossRegionUnsupported,
        InvalidRegion,
        NotFound,
        BrokenParentChain
    };


    struct PathResult
    {
        PathStatus status =
            PathStatus::NotFound;

        std::vector<TilePosition>
            tiles;

        int steps = 0;
        int turns = 0;
    };


    class Pathfinder
    {
    public:
        PathResult findPath(
            const World& world,
            const TilePosition& source,
            const TilePosition& destination
        ) const;
    };

}
