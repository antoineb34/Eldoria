#include "views/map/MapView.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#include "graphics/map/SceneLocationBuilder.h"
#include "graphics/map/SceneLocationModelBuilder.h"
#include "graphics/map/SceneMapRenderModelBuilder.h"
#include "render/scene/Transform.h"

namespace eld::elforge {

namespace {

constexpr float MapTileUnits = 128.0f;
constexpr float MapCenterTiles =
    static_cast<float>(eld::map::RegionSize) * 0.5f;

struct NeighborhoodRegion {
    int offsetX = 0;
    int offsetY = 0;
    eld::map::MapRegion region;
};

struct TerrainNeighborhood {
    std::vector<NeighborhoodRegion> regions;
    std::vector<std::uint16_t> missingRegionIds;
};

TerrainNeighborhood loadTerrainNeighborhood(
    const eld::map::MapLoader& loader,
    const eld::map::MapIndexEntry& center
) {
    TerrainNeighborhood result;
    result.regions.reserve(9);

    for (int offsetX = -1; offsetX <= 1; ++offsetX) {
        for (int offsetY = -1; offsetY <= 1; ++offsetY) {
            const int regionX = center.regionX() + offsetX;
            const int regionY = center.regionY() + offsetY;

            if (
                regionX < 0 || regionX > 255 ||
                regionY < 0 || regionY > 255
            ) {
                continue;
            }

            const std::uint16_t candidateId =
                static_cast<std::uint16_t>(
                    (regionX << 8) | regionY
                );

            if (loader.find(candidateId) == nullptr) {
                result.missingRegionIds.push_back(candidateId);
                continue;
            }

            result.regions.push_back({
                offsetX,
                offsetY,
                loader.loadTerrain(candidateId)
            });
        }
    }

    return result;
}

const eld::map::MapRegion* neighborhoodTerrain(
    const TerrainNeighborhood& neighborhood,
    int offsetX,
    int offsetY
) {
    for (
        const NeighborhoodRegion& region :
        neighborhood.regions
    ) {
        if (
            region.offsetX == offsetX &&
            region.offsetY == offsetY
        ) {
            return &region.region;
        }
    }

    return nullptr;
}

int regionOffsetForLocalCoordinate(
    int coordinate
) {
    if (coordinate >= 0) {
        return coordinate /
            static_cast<int>(eld::map::RegionSize);
    }

    return -(
        (-coordinate +
            static_cast<int>(eld::map::RegionSize) - 1) /
        static_cast<int>(eld::map::RegionSize)
    );
}

const eld::map::MapTile* sampleTerrainNeighborhood(
    const TerrainNeighborhood& neighborhood,
    std::size_t plane,
    int x,
    int y
) {
    if (plane >= eld::map::PlaneCount) {
        return nullptr;
    }

    const int offsetX =
        regionOffsetForLocalCoordinate(x);

    const int offsetY =
        regionOffsetForLocalCoordinate(y);

    if (
        offsetX < -1 || offsetX > 1 ||
        offsetY < -1 || offsetY > 1
    ) {
        return nullptr;
    }

    const eld::map::MapRegion* terrain =
        neighborhoodTerrain(
            neighborhood,
            offsetX,
            offsetY
        );

    if (terrain == nullptr) {
        return nullptr;
    }

    const int localX =
        x - offsetX *
            static_cast<int>(eld::map::RegionSize);

    const int localY =
        y - offsetY *
            static_cast<int>(eld::map::RegionSize);

    if (
        localX < 0 ||
        localX >= static_cast<int>(eld::map::RegionSize) ||
        localY < 0 ||
        localY >= static_cast<int>(eld::map::RegionSize)
    ) {
        return nullptr;
    }

    return &terrain->tile(
        plane,
        static_cast<std::size_t>(localX),
        static_cast<std::size_t>(localY)
    );
}

float terrainWorldHeight(
    int classicHeight
) {
    return -static_cast<float>(classicHeight) /
        MapTileUnits;
}

float averagePlaneHeight(
    const eld::map::MapRegion& region,
    std::size_t plane
) {
    std::int64_t sum = 0;
    std::size_t count = 0;

    for (
        std::size_t x = 0;
        x < eld::map::RegionSize;
        ++x
    ) {
        for (
            std::size_t y = 0;
            y < eld::map::RegionSize;
            ++y
        ) {
            sum += region.tile(plane, x, y).height;
            ++count;
        }
    }

    if (count == 0) {
        return 0.0f;
    }

    return terrainWorldHeight(
        static_cast<int>(
            sum / static_cast<std::int64_t>(count)
        )
    );
}

eld::render::Transform classicMapTransform() {
    eld::render::Transform transform;

    transform.position = {
        -MapCenterTiles,
        0.0f,
        -MapCenterTiles
    };

    transform.scale = {
        1.0f / MapTileUnits,
        1.0f / MapTileUnits,
        1.0f / MapTileUnits
    };

    return transform;
}

}

MapView::MapView(
    const eld::map::MapLoader& loader,
    const eld::definition::FloorRepository& floors,
    const eld::definition::LocationRepository& locations,
    eld::model::ModelRepository& models,
    eld::graphics::GraphicsResources& graphics
)
    : loader_(loader),
      floors_(floors),
      locations_(locations),
      models_(models),
      graphics_(graphics) {
}

MapViewState MapView::build(
    std::uint16_t regionId
) const {
    using Clock = std::chrono::steady_clock;

    const eld::map::MapIndexEntry* indexEntry =
        loader_.find(regionId);

    if (indexEntry == nullptr) {
        throw std::out_of_range(
            "map_index has no region " +
            std::to_string(regionId)
        );
    }

    const auto started = Clock::now();

    const TerrainNeighborhood neighborhood =
        loadTerrainNeighborhood(
            loader_,
            *indexEntry
        );

    const eld::map::MapRegion* centerTerrain =
        neighborhoodTerrain(
            neighborhood,
            0,
            0
        );

    if (centerTerrain == nullptr) {
        throw std::runtime_error(
            "selected map region has no terrain"
        );
    }

    MapViewState viewState;
    viewState.indexEntry = *indexEntry;
    viewState.centerRegion =
        loader_.load(regionId);

    viewState.missingNeighborRegionIds =
        neighborhood.missingRegionIds;

    const eld::graphics::map::TerrainTileSampler terrainSampler =
        [&](std::size_t plane, int x, int y) {
            return sampleTerrainNeighborhood(
                neighborhood,
                plane,
                x,
                y
            );
        };

    const eld::graphics::map::SceneLocationTileSampler locSampler =
        [&](std::size_t plane, int x, int y) {
            return sampleTerrainNeighborhood(
                neighborhood,
                plane,
                x,
                y
            );
        };

    eld::graphics::map::SceneLocationBuilder locBuilder;

    const std::vector<eld::graphics::map::SceneLocationPlacement>
        sceneLocs =
            locBuilder.build(
                viewState.centerRegion.locations,
                locations_,
                locSampler
            );

    viewState.sceneLocs = sceneLocs;

    eld::graphics::map::SceneLocationModelBuilder locModelBuilder;

    const eld::graphics::map::SceneLocationModelBuildResult
        locModels =
            locModelBuilder.build(
                sceneLocs,
                locations_,
                models_
            );

    std::vector<eld::graphics::ModelHandle>
        variantHandles;

    variantHandles.reserve(
        locModels.variants.size()
    );

    for (
        const eld::graphics::map::SceneLocationModelVariant& variant :
        locModels.variants
    ) {
        variantHandles.push_back(
            graphics_.resolveModel(
                variant.mesh
            )
        );
    }

    eld::graphics::map::SceneMapRenderModelBuilder
        renderModelBuilder;

    std::array<
        eld::graphics::map::SceneTerrainRenderBuildResult,
        eld::map::PlaneCount
    > terrainBuilds;

    for (
        std::size_t plane = 0;
        plane < eld::map::PlaneCount;
        ++plane
    ) {
        terrainBuilds[plane] =
            renderModelBuilder.buildTerrainPlane(
                plane,
                terrainSampler,
                floors_,
                graphics_
            );
    }

    eld::graphics::map::SceneLocationRenderBuildResult locRender =
        renderModelBuilder.buildLocs(
            locModels,
            variantHandles,
            graphics_
        );

    viewState.scene.camera.verticalFov = 0.96f;
    viewState.scene.camera.nearPlane = 0.5f;
    viewState.scene.camera.farPlane = 512.0f;

    const eld::render::Transform mapTransform =
        classicMapTransform();

    for (
        std::size_t plane = 0;
        plane < eld::map::PlaneCount;
        ++plane
    ) {
        const eld::graphics::ModelHandle terrainHandle =
            graphics_.registerModel(
                std::move(
                    terrainBuilds[plane].model
                )
            );

        const eld::graphics::ModelHandle locHandle =
            graphics_.registerModel(
                std::move(
                    locRender.staticPlaneModels[plane]
                )
            );

        viewState.terrainObjectIndices[plane] =
            viewState.scene.objects.size();

        viewState.scene.objects.push_back({
            terrainHandle,
            mapTransform,
            false
        });

        viewState.locObjectIndices[plane] =
            viewState.scene.objects.size();

        viewState.scene.objects.push_back({
            locHandle,
            mapTransform,
            false
        });

        viewState.averagePlaneHeights[plane] =
            averagePlaneHeight(
                *centerTerrain,
                plane
            );
    }

    viewState.cameraVariants.reserve(
        locRender.cameraVariants.size()
    );

    for (
        eld::graphics::map::SceneLocationCameraRenderVariant& variant :
        locRender.cameraVariants
    ) {
        const eld::graphics::ModelHandle insetHandle =
            graphics_.registerModel(
                std::move(variant.insetModel)
            );

        const eld::graphics::ModelHandle outsetHandle =
            graphics_.registerModel(
                std::move(variant.outsetModel)
            );

        const std::size_t insetObject =
            viewState.scene.objects.size();

        viewState.scene.objects.push_back({
            insetHandle,
            mapTransform,
            false
        });

        const std::size_t outsetObject =
            viewState.scene.objects.size();

        viewState.scene.objects.push_back({
            outsetHandle,
            mapTransform,
            false
        });

        viewState.cameraVariants.push_back({
            variant.scenePlane,
            variant.rotation,
            variant.sceneX,
            variant.sceneZ,
            insetObject,
            outsetObject
        });
    }

    viewState.stats.neighborhoodRegions =
        neighborhood.regions.size();

    viewState.stats.locPlacements =
        sceneLocs.size();

    viewState.stats.locModelInstances =
        locModels.stats.instances;

    viewState.stats.locModelParts =
        locModels.stats.parts;

    viewState.stats.locModelVariants =
        locModels.stats.variants;

    for (
        const auto& terrainBuild :
        terrainBuilds
    ) {
        viewState.stats.terrainTriangles +=
            terrainBuild.stats.triangles;

        viewState.stats.terrainDrawBuckets +=
            terrainBuild.stats.drawBuckets;
    }

    viewState.stats.locTriangles =
        locRender.stats.triangles;

    viewState.stats.locDrawBuckets =
        locRender.stats.staticDrawBuckets;

    viewState.stats.cameraDependentParts =
        locRender.stats.cameraDependentParts;

    viewState.stats.buildMilliseconds =
        std::chrono::duration<double, std::milli>(
            Clock::now() - started
        ).count();

    updateMapViewScene(
        viewState,
        0,
        true,
        true,
        0.75f,
        0.62f,
        82.0f,
        1,
        1
    );

    return viewState;
}

}
