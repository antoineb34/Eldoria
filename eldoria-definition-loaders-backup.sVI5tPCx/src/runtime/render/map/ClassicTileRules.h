#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "map/MapTile.h"

namespace eld::render::map {

class ClassicTileRules {
public:
    static constexpr bool isBridge(
        std::uint8_t levelOneSettings
    ) {
        return eld::map::hasTileFlag(
            levelOneSettings,
            eld::map::TileFlag::Bridge
        );
    }

    // RuneScape assigns each constructed Ground tile a draw level. 0x8 forces
    // level zero; otherwise the plane-1 bridge flag lowers upper levels by one.
    static constexpr int drawLevel(
        std::size_t decodedPlane,
        std::uint8_t decodedSettings,
        std::uint8_t levelOneSettings
    ) {
        if (
            eld::map::hasTileFlag(
                decodedSettings,
                eld::map::TileFlag::ForceLevelZero
            )
        ) {
            return 0;
        }

        if (
            decodedPlane > 0 &&
            isBridge(levelOneSettings)
        ) {
            return static_cast<int>(decodedPlane) - 1;
        }

        return static_cast<int>(decodedPlane);
    }

    // World3D::setBridge shifts the Ground containers at a bridge coordinate:
    // source plane 1 -> scene plane 0, 2 -> 1, 3 -> 2, and scene plane 3 is
    // emptied. The old source plane-0 Ground is retained as bridge geometry
    // attached beneath the new scene-plane-0 tile.
    static constexpr std::optional<std::size_t> scenePlaneForSourcePlane(
        std::size_t sourcePlane,
        std::uint8_t levelOneSettings
    ) {
        if (sourcePlane >= eld::map::PlaneCount) {
            return std::nullopt;
        }

        if (!isBridge(levelOneSettings)) {
            return sourcePlane;
        }

        // The old plane-0 Ground is retained as the bridge attachment at
        // scene plane 0, while source planes 1..3 shift down one level.
        if (sourcePlane == 0) {
            return 0;
        }

        return sourcePlane - 1;
    }

    static constexpr std::optional<std::size_t> sourcePlaneForScenePlane(
        std::size_t scenePlane,
        std::uint8_t levelOneSettings
    ) {
        if (!isBridge(levelOneSettings)) {
            if (scenePlane >= eld::map::PlaneCount) {
                return std::nullopt;
            }
            return scenePlane;
        }

        if (scenePlane >= eld::map::PlaneCount - 1) {
            return std::nullopt;
        }

        return scenePlane + 1;
    }

    static constexpr bool hasBridgeGround(
        std::size_t scenePlane,
        std::uint8_t levelOneSettings
    ) {
        return scenePlane == 0 && isBridge(levelOneSettings);
    }
};

}
