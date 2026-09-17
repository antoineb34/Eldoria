#pragma once

#include <cstddef>
#include <vector>

#include "Position.h"
#include "Tile.h"

namespace eld::world
{

    // One map tile is one World unit.
    inline constexpr float TileSize = 1.0f;

    struct TerrainCornerHeights
    {
        float southwest = 0.0f;
        float southeast = 0.0f;
        float northeast = 0.0f;
        float northwest = 0.0f;

        float average() const;
    };

    class Terrain
    {
    public:
        Terrain(
            TerrainOrigin origin,
            std::size_t width,
            std::size_t height,
            std::size_t sourcePlaneCount,
            std::vector<float> heightSamples,
            std::vector<Tile> tiles);

        const TerrainOrigin &origin() const;

        std::size_t width() const;
        std::size_t height() const;
        std::size_t sourcePlaneCount() const;

        // Convert region-local coordinates into an absolute
        // terrain-layer position.
        TerrainLayerPosition layerPosition(
            std::size_t sourcePlane,
            std::size_t localX,
            std::size_t localY) const;

        bool contains(
            const TerrainLayerPosition &position) const;

        const Tile &tile(
            const TerrainLayerPosition &position) const;

        TerrainCornerHeights cornerHeights(
            const TerrainLayerPosition &position) const;

        // Sample the terrain surface inside one tile.
        //
        // local.x:
        //   0 = west
        //   1 = east
        //
        // local.y:
        //   0 = south
        //   1 = north
        float heightAt(
            const TerrainLayerPosition &position,
            const TileLocalPosition &local) const;

    private:
        std::size_t localX(
            const TerrainLayerPosition &position) const;

        std::size_t localY(
            const TerrainLayerPosition &position) const;

        std::size_t tileIndex(
            const TerrainLayerPosition &position) const;

        std::size_t heightIndex(
            std::size_t sourcePlane,
            std::size_t x,
            std::size_t y) const;

        float heightSample(
            std::size_t sourcePlane,
            std::size_t x,
            std::size_t y) const;

        TerrainOrigin origin_{};

        std::size_t width_ = 0;
        std::size_t height_ = 0;
        std::size_t sourcePlaneCount_ = 0;

        // Shared corner heights.
        //
        // Per source plane:
        //
        //   (width + 1) x (height + 1)
        //
        std::vector<float> heightSamples_;

        // Tile cells.
        //
        // Per source plane:
        //
        //   width x height
        //
        std::vector<Tile> tiles_;
    };

}
