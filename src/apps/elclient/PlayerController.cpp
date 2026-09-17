#include "PlayerController.h"

#include <algorithm>
#include <cmath>


namespace eld::client
{

    PlayerController::PlayerController(
        Player& player,
        const eld::world::World& world,
        const eld::world::Pathfinder& pathfinder
    )
        : player_(
              player
          ),
          world_(
              world
          ),
          pathfinder_(
              pathfinder
          )
    {
    }


    bool PlayerController::moving() const noexcept
    {
        return moving_;
    }


    PlayerWalkResult
    PlayerController::walkTo(
        const eld::world::TilePosition& destination
    )
    {
        path_.clear();


        PlayerWalkResult
            result;


        result.source =
            moving_
                ? destinationTile_
                : player_.tile;


        result.path =
            pathfinder_.findPath(
                world_,
                result.source,
                destination
            );


        if (
            result.path.status !=
            eld::world::PathStatus::Success
        ) {
            return result;
        }


        for (
            const auto& tile :
            result.path.tiles
        ) {
            path_.push_back(
                tile
            );
        }


        if (!moving_) {
            continuePath();
        }


        return result;
    }


    bool PlayerController::continuePath()
    {
        if (
            moving_ ||
            path_.empty()
        ) {
            return false;
        }


        const auto next =
            path_.front();

        path_.pop_front();


        const int dx =
            static_cast<int>(
                next.x
            ) -
            static_cast<int>(
                player_.tile.x
            );

        const int dy =
            static_cast<int>(
                next.y
            ) -
            static_cast<int>(
                player_.tile.y
            );


        const bool adjacent =
            dx >= -1 &&
            dx <= 1 &&
            dy >= -1 &&
            dy <= 1 &&
            !(
                dx == 0 &&
                dy == 0
            );


        if (!adjacent) {
            path_.clear();
            return false;
        }


        return moveBy(
            dx,
            dy
        );
    }


    bool PlayerController::moveBy(
        int dx,
        int dy
    )
    {
        if (moving_) {
            return false;
        }


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


        const int destinationX =
            player_.tile.x +
            dx;

        const int destinationY =
            player_.tile.y +
            dy;


        const auto* destinationRegion =
            world_.regionAt({
                destinationX,
                destinationY,
                0
            });


        if (!destinationRegion) {
            return false;
        }


        const eld::world::TerrainLayerPosition
            terrainPosition{
                destinationX,
                destinationY,
                0
            };


        if (
            !destinationRegion
                ->terrain
                .contains(
                    terrainPosition
                )
        ) {
            return false;
        }


        const eld::world::TilePosition
            destination{
                destinationX,
                destinationY,
                player_.tile.plane
            };


        if (
            !world_.canMove(
                player_.tile,
                destination
            )
        ) {
            return false;
        }


        const auto& destinationTile =
            destinationRegion
                ->terrain
                .tile(
                    terrainPosition
                );


        setFacing(
            dx,
            dy
        );


        startTile_ =
            player_.tile;


        destinationTile_ = {
            destinationX,
            destinationY,
            destinationTile.scenePlane
        };


        elapsed_ =
            0.0f;

        moving_ =
            true;


        return true;
    }


    void PlayerController::setFacing(
        int dx,
        int dy
    )
    {
        if (
            dx > 0 &&
            dy > 0
        ) {
            player_.facing =
                FacingDirection::NorthEast;
        }
        else if (
            dx > 0 &&
            dy < 0
        ) {
            player_.facing =
                FacingDirection::SouthEast;
        }
        else if (
            dx < 0 &&
            dy > 0
        ) {
            player_.facing =
                FacingDirection::NorthWest;
        }
        else if (
            dx < 0 &&
            dy < 0
        ) {
            player_.facing =
                FacingDirection::SouthWest;
        }
        else if (dx > 0) {
            player_.facing =
                FacingDirection::East;
        }
        else if (dx < 0) {
            player_.facing =
                FacingDirection::West;
        }
        else if (dy > 0) {
            player_.facing =
                FacingDirection::North;
        }
        else {
            player_.facing =
                FacingDirection::South;
        }
    }


    PlayerMovementUpdate
    PlayerController::update(
        float dt
    )
    {
        PlayerMovementUpdate
            result;


        if (!moving_) {
            return result;
        }


        constexpr float moveDuration =
            0.6f;


        elapsed_ +=
            dt;


        const float progress =
            std::clamp(
                elapsed_ /
                    moveDuration,
                0.0f,
                1.0f
            );


        const float startX =
            static_cast<float>(
                startTile_.x
            ) +
            0.5f;

        const float startY =
            static_cast<float>(
                startTile_.y
            ) +
            0.5f;


        const float destinationX =
            static_cast<float>(
                destinationTile_.x
            ) +
            0.5f;

        const float destinationY =
            static_cast<float>(
                destinationTile_.y
            ) +
            0.5f;


        const float worldX =
            std::lerp(
                startX,
                destinationX,
                progress
            );

        const float worldY =
            std::lerp(
                startY,
                destinationY,
                progress
            );


        const int tileX =
            static_cast<int>(
                std::floor(
                    worldX
                )
            );

        const int tileY =
            static_cast<int>(
                std::floor(
                    worldY
                )
            );


        const auto* playerRegion =
            world_.regionAt({
                tileX,
                tileY,
                0
            });


        if (playerRegion) {
            const eld::world::TerrainLayerPosition
                terrainPosition{
                    tileX,
                    tileY,
                    0
                };


            if (
                playerRegion
                    ->terrain
                    .contains(
                        terrainPosition
                    )
            ) {
                const auto& tile =
                    playerRegion
                        ->terrain
                        .tile(
                            terrainPosition
                        );


                player_.tile = {
                    tileX,
                    tileY,
                    tile.scenePlane
                };


                player_.local = {
                    worldX -
                        static_cast<float>(
                            tileX
                        ),

                    worldY -
                        static_cast<float>(
                            tileY
                        )
                };


                result.positionChanged =
                    true;
            }
        }


        if (progress < 1.0f) {
            return result;
        }


        player_.tile =
            destinationTile_;

        player_.local = {
            0.5f,
            0.5f
        };


        moving_ =
            false;

        elapsed_ =
            0.0f;


        result.positionChanged =
            true;

        result.stepCompleted =
            true;


        return result;
    }

}
