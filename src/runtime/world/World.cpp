#include "World.h"

#include <algorithm>
#include <unordered_set>
#include <utility>


namespace eld::world
{

    namespace
    {

        int floorDiv64(
            int value
        )
        {
            if (value >= 0) {
                return
                    value /
                    World::RegionSize;
            }


            return
                -(
                    (
                        -value +
                        World::RegionSize -
                        1
                    ) /
                    World::RegionSize
                );
        }

    }


    RegionCoord World::regionCoordAt(
        const TilePosition& tile
    )
    {
        return {
            floorDiv64(
                tile.x
            ),

            floorDiv64(
                tile.y
            )
        };
    }


    std::optional<std::uint16_t>
    World::regionId(
        const RegionCoord& coord
    )
    {
        // RuneScape region ids encode each region axis
        // into one byte:
        //
        //     regionId =
        //         (regionX << 8) | regionY
        //
        // Therefore each axis must fit in [0, 255].

        if (
            coord.x < 0 ||
            coord.x > 255 ||
            coord.y < 0 ||
            coord.y > 255
        ) {
            return std::nullopt;
        }


        return static_cast<std::uint16_t>(
            (
                coord.x <<
                8
            ) |
            coord.y
        );
    }


    RegionCoord World::regionCoordFromId(
        std::uint16_t id
    )
    {
        return {
            static_cast<int>(
                (
                    id >>
                    8
                ) &
                0xff
            ),

            static_cast<int>(
                id &
                0xff
            )
        };
    }


    std::vector<std::uint16_t>
    World::desiredRegionIds(
        const RegionCoord& center
    )
    {
        std::vector<std::uint16_t>
            result;

        result.reserve(
            9
        );


        for (
            int dx = -1;
            dx <= 1;
            ++dx
        ) {
            for (
                int dy = -1;
                dy <= 1;
                ++dy
            ) {
                const RegionCoord coord{
                    center.x + dx,
                    center.y + dy
                };


                const auto id =
                    regionId(
                        coord
                    );


                if (!id.has_value()) {
                    continue;
                }


                result.push_back(
                    *id
                );
            }
        }


        return result;
    }


    WorldStreamingUpdate
    World::updateStreaming(
        const TilePosition& player
    )
    {
        WorldStreamingUpdate
            update;


        update.center =
            regionCoordAt(
                player
            );


        update.centerChanged =
            !center_.has_value() ||
            *center_ !=
                update.center;


        center_ =
            update.center;


        const auto desired =
            desiredRegionIds(
                update.center
            );


        std::unordered_set<
            std::uint16_t
        > desiredSet;

        desiredSet.reserve(
            desired.size()
        );


        for (
            const auto id :
            desired
        ) {
            desiredSet.insert(
                id
            );


            if (!loaded(id)) {
                update.toLoad.push_back(
                    id
                );
            }
        }


        for (
            const auto& [
                id,
                region
            ] :
            regions_
        ) {
            (void)region;


            if (
                !desiredSet.contains(
                    id
                )
            ) {
                update.toUnload.push_back(
                    id
                );
            }
        }


        std::sort(
            update.toLoad.begin(),
            update.toLoad.end()
        );


        std::sort(
            update.toUnload.begin(),
            update.toUnload.end()
        );


        return update;
    }


    void World::insertRegion(
        Region region
    )
    {
        const auto id =
            region.id;


        // Region does not need a default constructor.
        //
        // Erase first so emplacement works regardless of
        // whether Region supports assignment.
        regions_.erase(
            id
        );


        regions_.emplace(
            id,
            std::move(
                region
            )
        );
    }


    bool World::unloadRegion(
        std::uint16_t id
    )
    {
        return
            regions_.erase(
                id
            ) != 0;
    }


    bool World::loaded(
        std::uint16_t id
    ) const
    {
        return
            regions_.contains(
                id
            );
    }


    std::vector<std::uint16_t>
    World::loadedRegionIds() const
    {
        std::vector<std::uint16_t>
            result;

        result.reserve(
            regions_.size()
        );


        for (
            const auto& [
                id,
                region
            ] :
            regions_
        ) {
            (void)region;

            result.push_back(
                id
            );
        }


        std::sort(
            result.begin(),
            result.end()
        );


        return result;
    }


    Region* World::region(
        std::uint16_t id
    )
    {
        const auto it =
            regions_.find(
                id
            );


        if (
            it ==
            regions_.end()
        ) {
            return nullptr;
        }


        return
            &it->second;
    }


    const Region* World::region(
        std::uint16_t id
    ) const
    {
        const auto it =
            regions_.find(
                id
            );


        if (
            it ==
            regions_.end()
        ) {
            return nullptr;
        }


        return
            &it->second;
    }


    Region* World::regionAt(
        const TilePosition& tile
    )
    {
        const auto id =
            regionId(
                regionCoordAt(
                    tile
                )
            );


        if (!id.has_value()) {
            return nullptr;
        }


        return region(
            *id
        );
    }


    const Region* World::regionAt(
        const TilePosition& tile
    ) const
    {
        const auto id =
            regionId(
                regionCoordAt(
                    tile
                )
            );


        if (!id.has_value()) {
            return nullptr;
        }


        return region(
            *id
        );
    }


    Terrain* World::terrainAt(
        const TilePosition& tile
    )
    {
        auto* owner =
            regionAt(
                tile
            );


        if (!owner) {
            return nullptr;
        }


        return
            &owner->terrain;
    }


    const Terrain* World::terrainAt(
        const TilePosition& tile
    ) const
    {
        const auto* owner =
            regionAt(
                tile
            );


        if (!owner) {
            return nullptr;
        }


        return
            &owner->terrain;
    }


    bool World::canMove(
        const TilePosition& from,
        const TilePosition& to
    ) const
    {
        if (
            from.plane !=
            to.plane
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


        // For now, World deliberately preserves the old
        // single-region collision behavior exactly.
        //
        // Region seams come later, after this path has been
        // proven through the World abstraction.
        if (
            fromRegion !=
            toRegion
        ) {
            return false;
        }


        return
            fromRegion
                ->collision
                .canMove(
                    from,
                    to
                );
    }


}
