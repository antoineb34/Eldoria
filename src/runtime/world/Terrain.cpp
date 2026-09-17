#include "Terrain.h"

#include <stdexcept>
#include <utility>

namespace eld::world
{

    Terrain::Terrain(
        TerrainOrigin origin,
        std::size_t width,
        std::size_t height,
        std::size_t sourcePlaneCount,
        std::vector<float> heightSamples,
        std::vector<Tile> tiles)
        : origin_(origin),
          width_(width),
          height_(height),
          sourcePlaneCount_(sourcePlaneCount),
          heightSamples_(std::move(heightSamples)),
          tiles_(std::move(tiles))
    {

        const std::size_t expectedHeightSamples =
            sourcePlaneCount_ *
            (width_ + 1) *
            (height_ + 1);

        if (
            heightSamples_.size() !=
            expectedHeightSamples)
        {
            throw std::invalid_argument(
                "Terrain height sample count is invalid");
        }

        const std::size_t expectedTiles =
            sourcePlaneCount_ *
            width_ *
            height_;

        if (
            tiles_.size() !=
            expectedTiles)
        {
            throw std::invalid_argument(
                "Terrain tile count is invalid");
        }
    }

    const TerrainOrigin &
    Terrain::origin() const
    {
        return origin_;
    }

    std::size_t Terrain::width() const
    {
        return width_;
    }

    std::size_t Terrain::height() const
    {
        return height_;
    }

    std::size_t
    Terrain::sourcePlaneCount() const
    {
        return sourcePlaneCount_;
    }

    TerrainLayerPosition
    Terrain::layerPosition(
        std::size_t sourcePlane,
        std::size_t localXValue,
        std::size_t localYValue) const
    {
        if (
            sourcePlane >= sourcePlaneCount_ ||
            localXValue >= width_ ||
            localYValue >= height_)
        {
            throw std::out_of_range(
                "Terrain local position is outside terrain");
        }

        return {
            origin_.x +
                static_cast<int>(localXValue),

            origin_.y +
                static_cast<int>(localYValue),

            static_cast<int>(sourcePlane)};
    }

    bool Terrain::contains(
        const TerrainLayerPosition &position) const
    {
        return position.sourcePlane >= 0 &&

               position.sourcePlane <
                   static_cast<int>(
                       sourcePlaneCount_) &&

               position.x >= origin_.x &&
               position.y >= origin_.y &&

               position.x <
                   origin_.x +
                       static_cast<int>(width_) &&

               position.y <
                   origin_.y +
                       static_cast<int>(height_);
    }

    std::size_t Terrain::localX(
        const TerrainLayerPosition &position) const
    {
        if (!contains(position))
        {
            throw std::out_of_range(
                "Terrain position is outside terrain");
        }

        return static_cast<std::size_t>(
            position.x -
            origin_.x);
    }

    std::size_t Terrain::localY(
        const TerrainLayerPosition &position) const
    {
        if (!contains(position))
        {
            throw std::out_of_range(
                "Terrain position is outside terrain");
        }

        return static_cast<std::size_t>(
            position.y -
            origin_.y);
    }

    std::size_t Terrain::tileIndex(
        const TerrainLayerPosition &position) const
    {
        return static_cast<std::size_t>(
                   position.sourcePlane) *
                   width_ *
                   height_ +

               localX(position) *
                   height_ +

               localY(position);
    }

    std::size_t Terrain::heightIndex(
        std::size_t sourcePlane,
        std::size_t x,
        std::size_t y) const
    {
        if (
            sourcePlane >= sourcePlaneCount_ ||
            x > width_ ||
            y > height_)
        {
            throw std::out_of_range(
                "Terrain height sample is outside terrain");
        }

        const std::size_t sampleHeight =
            height_ + 1;

        return sourcePlane *
                   (width_ + 1) *
                   sampleHeight +

               x *
                   sampleHeight +

               y;
    }

    float Terrain::heightSample(
        std::size_t sourcePlane,
        std::size_t x,
        std::size_t y) const
    {
        return heightSamples_[heightIndex(
            sourcePlane,
            x,
            y)];
    }

    const Tile &Terrain::tile(
        const TerrainLayerPosition &position) const
    {
        return tiles_[tileIndex(position)];
    }

    TerrainCornerHeights
    Terrain::cornerHeights(
        const TerrainLayerPosition &position) const
    {
        const std::size_t x =
            localX(position);

        const std::size_t y =
            localY(position);

        const std::size_t plane =
            static_cast<std::size_t>(
                position.sourcePlane);

        return {
            .southwest =
                heightSample(
                    plane,
                    x,
                    y),

            .southeast =
                heightSample(
                    plane,
                    x + 1,
                    y),

            .northeast =
                heightSample(
                    plane,
                    x + 1,
                    y + 1),

            .northwest =
                heightSample(
                    plane,
                    x,
                    y + 1)};
    }

    float TerrainCornerHeights::average()
        const
    {
        return (
                   southwest +
                   southeast +
                   northeast +
                   northwest) *
               0.25f;
    }

    float Terrain::heightAt(
        const TerrainLayerPosition &position,
        const TileLocalPosition &local) const
    {
        if (
            local.x < 0.0f ||
            local.x > 1.0f ||
            local.y < 0.0f ||
            local.y > 1.0f)
        {
            throw std::out_of_range(
                "Tile-local position must be between 0 and 1");
        }

        const auto heights =
            cornerHeights(position);

        const float south =
            heights.southwest +
            (heights.southeast -
             heights.southwest) *
                local.x;

        const float north =
            heights.northwest +
            (heights.northeast -
             heights.northwest) *
                local.x;

        return south +
               (north - south) *
                   local.y;
    }

}
