#include "RegionProvider.h"


namespace eld::runtime::map
{

    RegionProvider::RegionProvider(
        const eld::map::MapLoader& maps,
        const eld::location::LocationLoader& locations
    )
        : maps_(maps),
          locations_(locations)
    {
    }


    eld::world::Region
    RegionProvider::load(
        std::uint16_t regionId
    ) const
    {
        return
            builder_.build(
                regionId,
                maps_,
                locations_
            );
    }

}
