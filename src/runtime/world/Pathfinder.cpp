#include "Pathfinder.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <queue>
#include <vector>


namespace eld::world
{

    PathResult Pathfinder::findPath(
        const World& world,
        const TilePosition& source,
        const TilePosition& destination
    ) const
    {
        PathResult result;


        if (
            source.plane !=
            destination.plane
        ) {
            result.status =
                PathStatus::PlaneMismatch;

            return result;
        }


        if (
            source.x == destination.x &&
            source.y == destination.y
        ) {
            result.status =
                PathStatus::SameTile;

            return result;
        }


        if (
            !world.regionAt(source) ||
            !world.regionAt(destination)
        ) {
            result.status =
                PathStatus::RegionMissing;

            return result;
        }


        // ----------------------------------------------------
        // Build a world-space search rectangle containing all
        // currently loaded Regions.
        //
        // Missing Regions inside the rectangle remain
        // unreachable because World::canMove() rejects them.
        // ----------------------------------------------------

        int minX =
            std::numeric_limits<int>::max();

        int minY =
            std::numeric_limits<int>::max();

        int maxX =
            std::numeric_limits<int>::min();

        int maxY =
            std::numeric_limits<int>::min();


        bool haveBounds =
            false;


        for (
            const auto id :
            world.loadedRegionIds()
        ) {
            const auto* region =
                world.region(
                    id
                );


            if (!region) {
                continue;
            }


            const auto& terrain =
                region->terrain;


            const int terrainWidth =
                static_cast<int>(
                    terrain.width()
                );

            const int terrainHeight =
                static_cast<int>(
                    terrain.height()
                );


            if (
                terrainWidth <= 0 ||
                terrainHeight <= 0
            ) {
                continue;
            }


            const auto origin =
                terrain.origin();


            minX =
                std::min(
                    minX,
                    origin.x
                );

            minY =
                std::min(
                    minY,
                    origin.y
                );


            maxX =
                std::max(
                    maxX,
                    origin.x +
                        terrainWidth
                );

            maxY =
                std::max(
                    maxY,
                    origin.y +
                        terrainHeight
                );


            haveBounds =
                true;
        }


        if (!haveBounds) {
            result.status =
                PathStatus::InvalidRegion;

            return result;
        }


        const int width =
            maxX -
            minX;

        const int height =
            maxY -
            minY;


        if (
            width <= 0 ||
            height <= 0
        ) {
            result.status =
                PathStatus::InvalidRegion;

            return result;
        }


        const auto inside =
            [=](
                int x,
                int y
            )
            {
                return
                    x >= minX &&
                    y >= minY &&
                    x < maxX &&
                    y < maxY;
            };


        if (
            !inside(
                source.x,
                source.y
            ) ||
            !inside(
                destination.x,
                destination.y
            )
        ) {
            result.status =
                PathStatus::InvalidRegion;

            return result;
        }


        constexpr int DirectionCount =
            8;

        constexpr int StartDirection =
            DirectionCount;

        constexpr int DirectionStateCount =
            DirectionCount +
            1;


        // Clockwise ordering:
        //
        // N, NE, E, SE, S, SW, W, NW
        //
        // Keep this exact order. It participates in
        // deterministic tie-breaking between otherwise equal
        // paths.
        constexpr int directionX[
            DirectionCount
        ] = {
             0,
             1,
             1,
             1,
             0,
            -1,
            -1,
            -1
        };


        constexpr int directionY[
            DirectionCount
        ] = {
             1,
             1,
             0,
            -1,
            -1,
            -1,
             0,
             1
        };


        const auto tileIndex =
            [=](
                int worldX,
                int worldY
            )
            {
                const int localX =
                    worldX -
                    minX;

                const int localY =
                    worldY -
                    minY;


                return
                    localY *
                        width +
                    localX;
            };


        const auto tilePosition =
            [=](
                int tile
            )
            {
                const int localX =
                    tile %
                    width;

                const int localY =
                    tile /
                    width;


                return TilePosition{
                    minX +
                        localX,

                    minY +
                        localY,

                    source.plane
                };
            };


        const auto stateIndex =
            [](
                int tile,
                int direction
            )
            {
                return
                    tile *
                        DirectionStateCount +
                    direction;
            };


        const int tileCount =
            width *
            height;

        const int stateCount =
            tileCount *
            DirectionStateCount;


        const int infinity =
            std::numeric_limits<int>::max();


        std::vector<int> bestSteps(
            static_cast<std::size_t>(
                stateCount
            ),
            infinity
        );

        std::vector<int> bestTurns(
            static_cast<std::size_t>(
                stateCount
            ),
            infinity
        );

        std::vector<int> parent(
            static_cast<std::size_t>(
                stateCount
            ),
            -1
        );


        struct SearchNode
        {
            int tile = 0;

            int direction =
                StartDirection;

            int steps = 0;
            int turns = 0;
        };


        struct CompareSearchNode
        {
            bool operator()(
                const SearchNode& a,
                const SearchNode& b
            ) const
            {
                if (
                    a.steps !=
                    b.steps
                ) {
                    return
                        a.steps >
                        b.steps;
                }


                if (
                    a.turns !=
                    b.turns
                ) {
                    return
                        a.turns >
                        b.turns;
                }


                return
                    a.direction >
                    b.direction;
            }
        };


        std::priority_queue<
            SearchNode,
            std::vector<SearchNode>,
            CompareSearchNode
        > open;


        const int sourceTile =
            tileIndex(
                source.x,
                source.y
            );

        const int destinationTile =
            tileIndex(
                destination.x,
                destination.y
            );


        const int sourceState =
            stateIndex(
                sourceTile,
                StartDirection
            );


        bestSteps[
            sourceState
        ] = 0;

        bestTurns[
            sourceState
        ] = 0;


        open.push({
            sourceTile,
            StartDirection,
            0,
            0
        });


        int finalState =
            -1;


        while (!open.empty()) {
            const auto current =
                open.top();

            open.pop();


            const int currentState =
                stateIndex(
                    current.tile,
                    current.direction
                );


            if (
                current.steps !=
                    bestSteps[
                        currentState
                    ] ||
                current.turns !=
                    bestTurns[
                        currentState
                    ]
            ) {
                continue;
            }


            if (
                current.tile ==
                destinationTile
            ) {
                finalState =
                    currentState;

                break;
            }


            const auto currentPosition =
                tilePosition(
                    current.tile
                );


            for (
                int direction = 0;
                direction <
                    DirectionCount;
                ++direction
            ) {
                const TilePosition nextPosition{
                    currentPosition.x +
                        directionX[
                            direction
                        ],

                    currentPosition.y +
                        directionY[
                            direction
                        ],

                    source.plane
                };


                if (
                    !inside(
                        nextPosition.x,
                        nextPosition.y
                    )
                ) {
                    continue;
                }


                if (
                    !world.canMove(
                        currentPosition,
                        nextPosition
                    )
                ) {
                    continue;
                }


                const int nextTile =
                    tileIndex(
                        nextPosition.x,
                        nextPosition.y
                    );

                const int nextState =
                    stateIndex(
                        nextTile,
                        direction
                    );


                const int nextSteps =
                    current.steps +
                    1;


                int nextTurns =
                    current.turns;


                if (
                    current.direction !=
                        StartDirection &&
                    current.direction !=
                        direction
                ) {
                    ++nextTurns;
                }


                const bool better =
                    nextSteps <
                        bestSteps[
                            nextState
                        ] ||
                    (
                        nextSteps ==
                            bestSteps[
                                nextState
                            ] &&
                        nextTurns <
                            bestTurns[
                                nextState
                            ]
                    );


                if (!better) {
                    continue;
                }


                bestSteps[
                    nextState
                ] =
                    nextSteps;

                bestTurns[
                    nextState
                ] =
                    nextTurns;

                parent[
                    nextState
                ] =
                    currentState;


                open.push({
                    nextTile,
                    direction,
                    nextSteps,
                    nextTurns
                });
            }
        }


        if (finalState < 0) {
            result.status =
                PathStatus::NotFound;

            return result;
        }


        std::vector<TilePosition>
            reversedPath;


        int state =
            finalState;


        while (
            state !=
            sourceState
        ) {
            const int tile =
                state /
                DirectionStateCount;


            reversedPath.push_back(
                tilePosition(
                    tile
                )
            );


            state =
                parent[
                    state
                ];


            if (state < 0) {
                result.status =
                    PathStatus::
                        BrokenParentChain;

                result.tiles.clear();

                return result;
            }
        }


        result.tiles.reserve(
            reversedPath.size()
        );


        for (
            auto it =
                reversedPath.rbegin();
            it !=
                reversedPath.rend();
            ++it
        ) {
            result.tiles.push_back(
                *it
            );
        }


        result.steps =
            bestSteps[
                finalState
            ];

        result.turns =
            bestTurns[
                finalState
            ];

        result.status =
            PathStatus::Success;


        return result;
    }

}
