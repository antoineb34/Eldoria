#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "Position.h"

namespace eld::world
{

    enum class CollisionFlag : std::uint16_t
    {
        Blocked = 1u << 0,

        North = 1u << 1,
        East = 1u << 2,
        South = 1u << 3,
        West = 1u << 4,

        NorthEast = 1u << 5,
        SouthEast = 1u << 6,
        SouthWest = 1u << 7,
        NorthWest = 1u << 8
    };


    class CollisionMap
    {
    public:
        CollisionMap() = default;


        CollisionMap(
            TerrainOrigin origin,
            std::size_t width,
            std::size_t height,
            std::size_t planeCount)
            : origin_(origin),
              width_(width),
              height_(height),
              planeCount_(planeCount),
              flags_(
                  width *
                  height *
                  planeCount,
                  0)
        {
        }


        bool contains(
            const TilePosition& position
        ) const
        {
            const int localX =
                position.x -
                origin_.x;

            const int localY =
                position.y -
                origin_.y;

            return
                position.plane >= 0 &&
                static_cast<std::size_t>(
                    position.plane
                ) < planeCount_ &&
                localX >= 0 &&
                localY >= 0 &&
                static_cast<std::size_t>(
                    localX
                ) < width_ &&
                static_cast<std::size_t>(
                    localY
                ) < height_;
        }


        void blockTile(
            const TilePosition& position
        )
        {
            addFlag(
                position,
                CollisionFlag::Blocked
            );
        }


        bool blocked(
            const TilePosition& position
        ) const
        {
            if (!contains(position)) {
                return true;
            }

            return (
                flags_.at(
                    index(position)
                ) &
                mask(
                    CollisionFlag::Blocked
                )
            ) != 0;
        }


        bool blocksEdge(
            const TilePosition& position,
            int dx,
            int dy
        ) const
        {
            if (!contains(position)) {
                return true;
            }

            return edgeBlocked(
                position,
                dx,
                dy
            );
        }


        void blockEdge(
            const TilePosition& position,
            int dx,
            int dy
        )
        {
            const auto forward =
                directionFlag(
                    dx,
                    dy
                );

            if (forward == 0) {
                return;
            }


            addMask(
                position,
                forward
            );


            TilePosition neighbor{
                position.x + dx,
                position.y + dy,
                position.plane
            };


            const auto backward =
                directionFlag(
                    -dx,
                    -dy
                );

            addMask(
                neighbor,
                backward
            );
        }


        bool canMove(
            const TilePosition& from,
            const TilePosition& to
        ) const
        {
            if (
                !contains(from) ||
                !contains(to) ||
                from.plane != to.plane
            ) {
                return false;
            }


            const int dx =
                to.x -
                from.x;

            const int dy =
                to.y -
                from.y;


            if (
                dx < -1 ||
                dx > 1 ||
                dy < -1 ||
                dy > 1 ||
                (
                    dx == 0 &&
                    dy == 0
                )
            ) {
                return false;
            }


            if (blocked(to)) {
                return false;
            }


            // Cardinal movement.
            if (
                dx == 0 ||
                dy == 0
            ) {
                return !edgeBlocked(
                    from,
                    dx,
                    dy
                );
            }


            // A diagonal wall can directly block the diagonal.
            if (
                edgeBlocked(
                    from,
                    dx,
                    dy
                )
            ) {
                return false;
            }


            // Don't cut diagonally through blocked tiles.
            const TilePosition horizontal{
                from.x + dx,
                from.y,
                from.plane
            };

            const TilePosition vertical{
                from.x,
                from.y + dy,
                from.plane
            };


            if (
                blocked(horizontal) ||
                blocked(vertical)
            ) {
                return false;
            }


            // Both cardinal exits from the source must be open.
            if (
                edgeBlocked(
                    from,
                    dx,
                    0
                ) ||
                edgeBlocked(
                    from,
                    0,
                    dy
                )
            ) {
                return false;
            }


            // And both approaches into the diagonal destination
            // must also be open.
            if (
                edgeBlocked(
                    horizontal,
                    0,
                    dy
                ) ||
                edgeBlocked(
                    vertical,
                    dx,
                    0
                )
            ) {
                return false;
            }


            return true;
        }


    private:
        TerrainOrigin origin_{};

        std::size_t width_ = 0;
        std::size_t height_ = 0;
        std::size_t planeCount_ = 0;

        std::vector<std::uint16_t>
            flags_;


        static constexpr std::uint16_t mask(
            CollisionFlag flag
        )
        {
            return static_cast<
                std::uint16_t
            >(flag);
        }


        static constexpr std::uint16_t
        directionFlag(
            int dx,
            int dy
        )
        {
            if (
                dx == 0 &&
                dy == 1
            ) {
                return mask(
                    CollisionFlag::North
                );
            }

            if (
                dx == 1 &&
                dy == 0
            ) {
                return mask(
                    CollisionFlag::East
                );
            }

            if (
                dx == 0 &&
                dy == -1
            ) {
                return mask(
                    CollisionFlag::South
                );
            }

            if (
                dx == -1 &&
                dy == 0
            ) {
                return mask(
                    CollisionFlag::West
                );
            }

            if (
                dx == 1 &&
                dy == 1
            ) {
                return mask(
                    CollisionFlag::NorthEast
                );
            }

            if (
                dx == 1 &&
                dy == -1
            ) {
                return mask(
                    CollisionFlag::SouthEast
                );
            }

            if (
                dx == -1 &&
                dy == -1
            ) {
                return mask(
                    CollisionFlag::SouthWest
                );
            }

            if (
                dx == -1 &&
                dy == 1
            ) {
                return mask(
                    CollisionFlag::NorthWest
                );
            }

            return 0;
        }


        std::size_t index(
            const TilePosition& position
        ) const
        {
            const auto localX =
                static_cast<std::size_t>(
                    position.x -
                    origin_.x
                );

            const auto localY =
                static_cast<std::size_t>(
                    position.y -
                    origin_.y
                );

            const auto plane =
                static_cast<std::size_t>(
                    position.plane
                );

            return
                plane *
                    width_ *
                    height_ +
                localY *
                    width_ +
                localX;
        }


        void addFlag(
            const TilePosition& position,
            CollisionFlag flag
        )
        {
            addMask(
                position,
                mask(flag)
            );
        }


        void addMask(
            const TilePosition& position,
            std::uint16_t value
        )
        {
            if (!contains(position)) {
                return;
            }

            flags_.at(
                index(position)
            ) |= value;
        }


        bool edgeBlocked(
            const TilePosition& from,
            int dx,
            int dy
        ) const
        {
            const auto forward =
                directionFlag(
                    dx,
                    dy
                );

            if (forward == 0) {
                return true;
            }


            const TilePosition to{
                from.x + dx,
                from.y + dy,
                from.plane
            };


            if (!contains(to)) {
                return true;
            }


            const auto backward =
                directionFlag(
                    -dx,
                    -dy
                );


            return
                (
                    flags_.at(
                        index(from)
                    ) &
                    forward
                ) != 0 ||
                (
                    flags_.at(
                        index(to)
                    ) &
                    backward
                ) != 0;
        }
    };

}
