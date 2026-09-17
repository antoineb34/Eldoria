#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "RegionProvider.h"
#include "world/Position.h"
#include "world/World.h"


namespace eld::runtime::map
{

    struct WorldStreamingFailure
    {
        std::uint16_t regionId = 0;
        std::string message;
    };


    struct WorldStreamingResult
    {
        eld::world::WorldStreamingUpdate
            plan;

        std::vector<std::uint16_t>
            loaded;

        std::vector<std::uint16_t>
            unloaded;

        std::vector<WorldStreamingFailure>
            failures;
    };


    class WorldStreamer
    {
    public:
        WorldStreamer(
            eld::world::World& world,
            const eld::map::MapLoader& maps,
            const eld::location::LocationLoader& locations
        );


        WorldStreamingResult update(
            const eld::world::TilePosition& player
        );


    private:
        eld::world::World&
            world_;

        RegionProvider
            provider_;
    };

}
