#pragma once

#include <deque>

#include "Player.h"
#include "world/Pathfinder.h"
#include "world/World.h"


namespace eld::client
{

    struct PlayerWalkResult
    {
        eld::world::TilePosition
            source{};

        eld::world::PathResult
            path{};
    };


    struct PlayerMovementUpdate
    {
        bool positionChanged =
            false;

        bool stepCompleted =
            false;
    };


    class PlayerController
    {
    public:
        PlayerController(
            Player& player,
            const eld::world::World& world,
            const eld::world::Pathfinder& pathfinder
        );


        PlayerWalkResult walkTo(
            const eld::world::TilePosition& destination
        );


        bool moveBy(
            int dx,
            int dy
        );


        PlayerMovementUpdate update(
            float dt
        );


        bool continuePath();


        bool moving() const noexcept;


    private:
        void setFacing(
            int dx,
            int dy
        );


        Player&
            player_;

        const eld::world::World&
            world_;

        const eld::world::Pathfinder&
            pathfinder_;


        std::deque<
            eld::world::TilePosition
        > path_;


        bool moving_ =
            false;


        eld::world::TilePosition
            startTile_{};

        eld::world::TilePosition
            destinationTile_{};


        float elapsed_ =
            0.0f;
    };

}
