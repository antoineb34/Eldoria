#include "Pathfinder.h"

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


        const auto* sourceRegion =
            world.regionAt(
                source
            );

        const auto* destinationRegion =
            world.regionAt(
                destination
            );


        if (
            !sourceRegion ||
            !destinationRegion
        ) {
            result.status =
                PathStatus::RegionMissing;

            return result;
        }


        // Current recovery checkpoint:
        //
        // preserve the known-good single-region search until
        // cross-region CollisionMap semantics are implemented.
        if (
            sourceRegion !=
            destinationRegion
        ) {
            result.status =
                PathStatus::CrossRegionUnsupported;

            return result;
        }


        const auto& terrain =
            sourceRegion->terrain;

        const auto origin =
            terrain.origin();


        const int width =
            static_cast<int>(
                terrain.width()
            );

        const int height =
            static_cast<int>(
                terrain.height()
            );


        if (
            width <= 0 ||
            height <= 0
        ) {
            result.status =
                PathStatus::InvalidRegion;

            return result;
        }


        const int sourceX =
            source.x -
            origin.x;

        const int sourceY =
            source.y -
            origin.y;

        const int destinationX =
            destination.x -
            origin.x;

        const int destinationY =
            destination.y -
            origin.y;


        const auto inside =
            [width, height](
                int x,
                int y
            )
            {
                return
                    x >= 0 &&
                    y >= 0 &&
                    x < width &&
                    y < height;
            };


        if (
            !inside(
                sourceX,
                sourceY
            ) ||
            !inside(
                destinationX,
                destinationY
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
            DirectionCount + 1;


        // Clockwise ordering:
        //
        // N, NE, E, SE, S, SW, W, NW
        //
        // This order is intentionally preserved because it
        // breaks otherwise-identical search ties.
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
            [width](
                int localX,
                int localY
            )
            {
                return
                    localY *
                        width +
                    localX;
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
                sourceX,
                sourceY
            );

        const int destinationTile =
            tileIndex(
                destinationX,
                destinationY
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


            const int currentLocalX =
                current.tile %
                width;

            const int currentLocalY =
                current.tile /
                width;


            const TilePosition
                currentPosition{
                    origin.x +
                        currentLocalX,

                    origin.y +
                        currentLocalY,

                    source.plane
                };


            for (
                int direction = 0;
                direction <
                    DirectionCount;
                ++direction
            ) {
                const int nextLocalX =
                    currentLocalX +
                    directionX[
                        direction
                    ];

                const int nextLocalY =
                    currentLocalY +
                    directionY[
                        direction
                    ];


                if (
                    !inside(
                        nextLocalX,
                        nextLocalY
                    )
                ) {
                    continue;
                }


                const TilePosition
                    nextPosition{
                        origin.x +
                            nextLocalX,

                        origin.y +
                            nextLocalY,

                        source.plane
                    };


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
                        nextLocalX,
                        nextLocalY
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

            const int localX =
                tile %
                width;

            const int localY =
                tile /
                width;


            reversedPath.push_back({
                origin.x +
                    localX,

                origin.y +
                    localY,

                source.plane
            });


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
