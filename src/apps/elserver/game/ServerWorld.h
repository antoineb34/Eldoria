#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>

#include "location/LocationLoader.h"
#include "map/MapLoader.h"
#include "map/RegionProvider.h"
#include "world/Position.h"
#include "world/Region.h"


namespace eld::elserver
{

    class ServerWorld
    {
    public:
        static constexpr int
            RegionSize =
                64;


        ServerWorld(
            const eld::map::MapLoader& maps,
            const eld::location::LocationLoader& locations
        );


        void loadAround(
            const eld::world::TilePosition& center,
            int radius
        );


        bool loaded(
            std::uint16_t regionId
        ) const;


        std::size_t loadedRegionCount() const noexcept;


        eld::world::Region* regionAt(
            const eld::world::TilePosition& tile
        );


        const eld::world::Region* regionAt(
            const eld::world::TilePosition& tile
        ) const;


        bool canMove(
            const eld::world::TilePosition& from,
            const eld::world::TilePosition& to
        ) const;


    private:
        static std::optional<std::uint16_t>
        regionIdAt(
            const eld::world::TilePosition& tile
        );


        bool loadRegion(
            std::uint16_t regionId
        );


        eld::runtime::map::RegionProvider
            provider_;


        std::unordered_map<
            std::uint16_t,
            eld::world::Region
        >
            regions_;
    };

}
