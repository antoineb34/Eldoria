#pragma once

#include <cstdint>

#include "location/LocationLoader.h"
#include "map/MapLoader.h"
#include "RegionBuilder.h"
#include "world/Region.h"


namespace eld::runtime::map
{

    class RegionProvider
    {
    public:
        RegionProvider(
            const eld::map::MapLoader& maps,
            const eld::location::LocationLoader& locations
        );


        eld::world::Region load(
            std::uint16_t regionId
        ) const;


    private:
        const eld::map::MapLoader&
            maps_;

        const eld::location::LocationLoader&
            locations_;

        RegionBuilder
            builder_;
    };

}
