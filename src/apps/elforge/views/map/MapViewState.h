#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "map/MapIndex.h"
#include "map/MapRegion.h"
#include "render/map/SceneLocationBuilder.h"
#include "render/scene/RenderScene.h"

namespace eld::elforge {

struct MapViewCameraVariant {
    std::uint8_t scenePlane = 0;
    std::uint8_t rotation = 0;
    int sceneX = 0;
    int sceneZ = 0;
    std::size_t insetObject = 0;
    std::size_t outsetObject = 0;
};

struct MapViewStats {
    std::size_t neighborhoodRegions = 0;
    std::size_t locPlacements = 0;
    std::size_t locModelInstances = 0;
    std::size_t locModelParts = 0;
    std::size_t locModelVariants = 0;
    std::size_t terrainTriangles = 0;
    std::size_t terrainDrawBuckets = 0;
    std::size_t locTriangles = 0;
    std::size_t locDrawBuckets = 0;
    std::size_t cameraDependentParts = 0;
    double buildMilliseconds = 0.0;
};

struct MapViewState {
    eld::map::MapIndexEntry indexEntry;
    eld::map::MapRegion centerRegion;

    std::vector<eld::render::map::SceneLocationPlacement>
        sceneLocs;

    std::array<float, eld::map::PlaneCount>
        averagePlaneHeights{};

    eld::render::RenderScene scene;

    std::array<std::size_t, eld::map::PlaneCount>
        terrainObjectIndices{};

    std::array<std::size_t, eld::map::PlaneCount>
        locObjectIndices{};

    std::vector<MapViewCameraVariant>
        cameraVariants;

    std::vector<std::uint16_t>
        missingNeighborRegionIds;

    MapViewStats stats;
};

void resetMapView(
    std::size_t& plane,
    float& yaw,
    float& pitch,
    float& distance
);

void updateMapViewScene(
    MapViewState& viewState,
    std::size_t plane,
    bool showTerrain,
    bool showLocs,
    float yaw,
    float pitch,
    float distance,
    std::uint32_t viewportWidth,
    std::uint32_t viewportHeight
);

}
