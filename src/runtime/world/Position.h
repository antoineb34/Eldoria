#pragma once

#include <cstdint>

namespace eld::world
{

    enum class QuarterTurn : std::uint8_t
    {
        Zero = 0,
        One = 1,
        Two = 2,
        Three = 3
    };

    constexpr std::uint8_t quarterTurns(
        QuarterTurn rotation)
    {
        return static_cast<std::uint8_t>(
            rotation);
    }

    struct TilePosition
    {
        int x = 0;
        int y = 0;

        // Effective World plane.
        int plane = 0;

        bool operator==(
            const TilePosition &) const = default;
    };

    struct TerrainLayerPosition
    {
        int x = 0;
        int y = 0;

        // Source terrain layer before bridge-plane projection.
        int sourcePlane = 0;

        bool operator==(
            const TerrainLayerPosition &) const = default;
    };

    struct TileLocalPosition
    {
        // Normalized position inside one tile:
        //
        // x: 0 west -> 1 east
        // y: 0 south -> 1 north
        float x = 0.0f;
        float y = 0.0f;

        bool operator==(
            const TileLocalPosition &) const = default;
    };

    struct WorldPosition
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        bool operator==(
            const WorldPosition &) const = default;
    };

    struct TerrainOrigin
    {
        // Absolute tile coordinate of this region's local (0,0).
        int x = 0;
        int y = 0;

        bool operator==(
            const TerrainOrigin &) const = default;
    };

}
