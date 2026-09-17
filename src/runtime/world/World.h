#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "Position.h"
#include "Region.h"


namespace eld::world
{

    struct RegionCoord
    {
        int x = 0;
        int y = 0;


        bool operator==(
            const RegionCoord&
        ) const = default;
    };


    struct WorldStreamingUpdate
    {
        bool centerChanged =
            false;

        RegionCoord center{};


        std::vector<std::uint16_t>
            toLoad;

        std::vector<std::uint16_t>
            toUnload;
    };


    class World
    {
    public:
        static constexpr int
            RegionSize = 64;


        // ----------------------------------------------------
        // Region coordinate helpers
        // ----------------------------------------------------

        static RegionCoord regionCoordAt(
            const TilePosition& tile
        );


        static std::optional<std::uint16_t>
        regionId(
            const RegionCoord& coord
        );


        static RegionCoord regionCoordFromId(
            std::uint16_t id
        );


        static std::vector<std::uint16_t>
        desiredRegionIds(
            const RegionCoord& center
        );


        // ----------------------------------------------------
        // Streaming
        //
        // This performs no decoding and no rendering.
        //
        // It simply answers:
        //
        //     which regions are missing?
        //     which regions are no longer needed?
        //
        // The caller can then load/unload them.
        // ----------------------------------------------------

        WorldStreamingUpdate updateStreaming(
            const TilePosition& player
        );


        // ----------------------------------------------------
        // Runtime region ownership
        // ----------------------------------------------------

        void insertRegion(
            Region region
        );


        bool unloadRegion(
            std::uint16_t id
        );


        bool loaded(
            std::uint16_t id
        ) const;


        std::size_t loadedRegionCount() const
        {
            return regions_.size();
        }


        std::vector<std::uint16_t>
        loadedRegionIds() const;


        // ----------------------------------------------------
        // Region lookup
        // ----------------------------------------------------

        Region* region(
            std::uint16_t id
        );


        const Region* region(
            std::uint16_t id
        ) const;


        Region* regionAt(
            const TilePosition& tile
        );


        const Region* regionAt(
            const TilePosition& tile
        ) const;


        // ----------------------------------------------------
        // World queries
        // ----------------------------------------------------

        Terrain* terrainAt(
            const TilePosition& tile
        );


        const Terrain* terrainAt(
            const TilePosition& tile
        ) const;


        bool canMove(
            const TilePosition& from,
            const TilePosition& to
        ) const;


        const std::optional<RegionCoord>&
        streamingCenter() const
        {
            return center_;
        }


    private:
        std::unordered_map<
            std::uint16_t,
            Region
        > regions_;


        std::optional<RegionCoord>
            center_;
    };

}
