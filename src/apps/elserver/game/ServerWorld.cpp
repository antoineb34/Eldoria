#include "game/ServerWorld.h"

#include <exception>
#include <iostream>
#include <utility>


namespace eld::elserver
{

    namespace
    {

        int floorDiv64(
            int value
        )
        {
            if (
                value >=
                0
            ) {
                return
                    value /
                    ServerWorld::RegionSize;
            }


            return
                -(
                    (
                        -value +
                        ServerWorld::RegionSize -
                        1
                    ) /
                    ServerWorld::RegionSize
                );
        }


        std::optional<std::uint16_t>
        makeRegionId(
            int regionX,
            int regionY
        )
        {
            if (
                regionX < 0 ||
                regionX > 255 ||
                regionY < 0 ||
                regionY > 255
            ) {
                return
                    std::nullopt;
            }


            return
                static_cast<std::uint16_t>(
                    (
                        regionX <<
                        8
                    ) |
                    regionY
                );
        }

    }


    ServerWorld::ServerWorld(
        const eld::map::MapLoader& maps,
        const eld::location::LocationLoader& locations
    )
        : provider_(
              maps,
              locations
          )
    {
    }


    std::optional<std::uint16_t>
    ServerWorld::regionIdAt(
        const eld::world::TilePosition& tile
    )
    {
        return
            makeRegionId(
                floorDiv64(
                    tile.x
                ),

                floorDiv64(
                    tile.y
                )
            );
    }


    bool ServerWorld::loaded(
        std::uint16_t regionId
    ) const
    {
        return
            regions_.contains(
                regionId
            );
    }


    std::size_t
    ServerWorld::loadedRegionCount() const noexcept
    {
        return
            regions_.size();
    }


    bool ServerWorld::loadRegion(
        std::uint16_t regionId
    )
    {
        if (
            loaded(
                regionId
            )
        ) {
            return true;
        }


        try {
            auto region =
                provider_.load(
                    regionId
                );


            regions_.emplace(
                regionId,
                std::move(
                    region
                )
            );


            std::cout
                << "server world | loaded region | "
                << regionId
                << "\n";


            return true;
        }
        catch (
            const std::exception& error
        ) {
            std::cerr
                << "server world | failed region | "
                << regionId
                << " | "
                << error.what()
                << "\n";


            return false;
        }
    }


    void ServerWorld::loadAround(
        const eld::world::TilePosition& center,
        int radius
    )
    {
        if (
            radius < 0
        ) {
            return;
        }


        const int centerRegionX =
            floorDiv64(
                center.x
            );


        const int centerRegionY =
            floorDiv64(
                center.y
            );


        for (
            int dx = -radius;
            dx <= radius;
            ++dx
        ) {
            for (
                int dy = -radius;
                dy <= radius;
                ++dy
            ) {
                const auto regionId =
                    makeRegionId(
                        centerRegionX + dx,
                        centerRegionY + dy
                    );


                if (
                    !regionId.has_value()
                ) {
                    continue;
                }


                loadRegion(
                    *regionId
                );
            }
        }
    }


    eld::world::Region*
    ServerWorld::regionAt(
        const eld::world::TilePosition& tile
    )
    {
        const auto regionId =
            regionIdAt(
                tile
            );


        if (
            !regionId.has_value()
        ) {
            return nullptr;
        }


        const auto iterator =
            regions_.find(
                *regionId
            );


        if (
            iterator ==
            regions_.end()
        ) {
            return nullptr;
        }


        return
            &iterator->second;
    }


    const eld::world::Region*
    ServerWorld::regionAt(
        const eld::world::TilePosition& tile
    ) const
    {
        const auto regionId =
            regionIdAt(
                tile
            );


        if (
            !regionId.has_value()
        ) {
            return nullptr;
        }


        const auto iterator =
            regions_.find(
                *regionId
            );


        if (
            iterator ==
            regions_.end()
        ) {
            return nullptr;
        }


        return
            &iterator->second;
    }


    bool ServerWorld::canMove(
        const eld::world::TilePosition& from,
        const eld::world::TilePosition& to
    ) const
    {
        if (
            from.plane !=
            to.plane
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


        const auto* fromRegion =
            regionAt(
                from
            );


        const auto* toRegion =
            regionAt(
                to
            );


        if (
            !fromRegion ||
            !toRegion
        ) {
            return false;
        }


        if (
            toRegion
                ->collision
                .blocked(
                    to
                )
        ) {
            return false;
        }


        const auto tileBlocked =
            [this](
                const eld::world::TilePosition& tile
            )
            {
                const auto* region =
                    regionAt(
                        tile
                    );


                return
                    !region ||
                    region
                        ->collision
                        .blocked(
                            tile
                        );
            };


        const auto edgeBlocked =
            [this](
                const eld::world::TilePosition& a,
                const eld::world::TilePosition& b
            )
            {
                const auto* aRegion =
                    regionAt(
                        a
                    );


                const auto* bRegion =
                    regionAt(
                        b
                    );


                if (
                    !aRegion ||
                    !bRegion
                ) {
                    return true;
                }


                const int edgeX =
                    b.x -
                    a.x;


                const int edgeY =
                    b.y -
                    a.y;


                return
                    aRegion
                        ->collision
                        .blocksOwnEdge(
                            a,
                            edgeX,
                            edgeY
                        ) ||
                    bRegion
                        ->collision
                        .blocksOwnEdge(
                            b,
                            -edgeX,
                            -edgeY
                        );
            };


        // Cardinal movement.
        if (
            dx == 0 ||
            dy == 0
        ) {
            return
                !edgeBlocked(
                    from,
                    to
                );
        }


        // Direct diagonal wall.
        if (
            edgeBlocked(
                from,
                to
            )
        ) {
            return false;
        }


        const eld::world::TilePosition
            horizontal{
                from.x + dx,
                from.y,
                from.plane
            };


        const eld::world::TilePosition
            vertical{
                from.x,
                from.y + dy,
                from.plane
            };


        // Don't cut through blocked cardinal neighbors.
        if (
            tileBlocked(
                horizontal
            ) ||
            tileBlocked(
                vertical
            )
        ) {
            return false;
        }


        // Source must be able to leave in both cardinal
        // directions.
        if (
            edgeBlocked(
                from,
                horizontal
            ) ||
            edgeBlocked(
                from,
                vertical
            )
        ) {
            return false;
        }


        // Both cardinal neighbors must be able to approach
        // the diagonal destination.
        if (
            edgeBlocked(
                horizontal,
                to
            ) ||
            edgeBlocked(
                vertical,
                to
            )
        ) {
            return false;
        }


        return true;
    }

}
