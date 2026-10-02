#include "WorldScene.h"

#include <utility>

#include "map/LocationBatchBuilder.h"
#include "map/LocationBuilder.h"
#include "map/TerrainBuilder.h"
#include "render/scene/RenderObject.h"


namespace eld::client
{

    WorldScene::WorldScene(
        const eld::floor::FloorLoader& floors,
        const eld::location::LocationLoader& locations,
        eld::model::ModelLoader& models,
        eld::graphics::TextureSystem& textureSystem,
        eld::graphics::ModelSystem& modelSystem,
        eld::render::ModelManager& modelManager
    )
        : floors_(
              floors
          ),
          locations_(
              locations
          ),
          models_(
              models
          ),
          textureSystem_(
              textureSystem
          ),
          modelSystem_(
              modelSystem
          ),
          modelManager_(
              modelManager
          )
    {
    }


    WorldScene::~WorldScene()
    {
        reset();
    }


    std::size_t
    WorldScene::renderedRegionCount() const noexcept
    {
        return
            renderedRegions_.size();
    }


    void WorldScene::destroyOwnedModels(
        RenderedRegion& rendered
    )
    {
        for (
            const auto handle :
            rendered.ownedModels
        ) {
            if (
                modelManager_.isValid(
                    handle
                )
            ) {
                modelManager_.destroy(
                    handle
                );
            }
        }


        rendered.ownedModels.clear();
    }


    void WorldScene::reset()
    {
        for (
            auto& [regionId, rendered] :
            renderedRegions_
        ) {
            (void)regionId;

            destroyOwnedModels(
                rendered
            );
        }


        renderedRegions_.clear();
        freeObjectSlots_.clear();
    }


    std::size_t WorldScene::placeObject(
        eld::render::RenderObject object,
        eld::render::RenderScene& scene
    )
    {
        while (
            !freeObjectSlots_.empty()
        ) {
            const std::size_t slot =
                freeObjectSlots_.back();

            freeObjectSlots_.pop_back();


            // If somebody externally rebuilt RenderScene,
            // silently discard stale reusable slots.
            if (
                slot >=
                scene.objects.size()
            ) {
                continue;
            }


            scene.objects[
                slot
            ] =
                std::move(
                    object
                );


            return slot;
        }


        const std::size_t slot =
            scene.objects.size();


        scene.objects.push_back(
            std::move(
                object
            )
        );


        return slot;
    }


    void WorldScene::loadRegion(
        const eld::world::Region& region,
        const eld::world::TerrainOrigin& sceneOrigin,
        eld::render::RenderScene& scene
    )
    {
        if (
            renderedRegions_.contains(
                region.id
            )
        ) {
            return;
        }


        RenderedRegion rendered;


        const auto& origin =
            region
                .terrain
                .origin();


        const float sceneOffsetX =
            static_cast<float>(
                origin.x -
                sceneOrigin.x
            );


        const float sceneOffsetZ =
            -static_cast<float>(
                origin.y -
                sceneOrigin.y
            );


        // ----------------------------------------------------
        // Terrain
        // ----------------------------------------------------

        eld::graphics::TerrainBuilder
            terrainBuilder;


        auto terrainModel =
            terrainBuilder.build(
                region.terrain,
                0,
                floors_,
                textureSystem_
            );


        const auto terrainHandle =
            modelManager_.create(
                std::move(
                    terrainModel
                )
            );


        rendered.ownedModels.push_back(
            terrainHandle
        );


        eld::render::RenderObject
            terrainObject;


        terrainObject.model =
            terrainHandle;


        terrainObject.transform.position = {
            sceneOffsetX,
            0.0f,
            sceneOffsetZ
        };


        terrainObject.visible =
            true;


        rendered.objectSlots.push_back(
            placeObject(
                std::move(
                    terrainObject
                ),
                scene
            )
        );


        // ----------------------------------------------------
        // Locations
        // ----------------------------------------------------

        eld::graphics::LocationBuilder
            locationBuilder;


        auto locationBuild =
            locationBuilder.build(
                region,
                0,
                locations_,
                models_,
                modelSystem_
            );


        eld::graphics::LocationBatchBuilder
            locationBatchBuilder;


        auto locationBatchBuild =
            locationBatchBuilder.build(
                locationBuild.objects,
                modelManager_
            );


        // Location batches create new ModelResources owned by
        // this rendered Region.
        for (
            auto& batch :
            locationBatchBuild.batches
        ) {
            const auto handle =
                modelManager_.create(
                    std::move(
                        batch
                    )
                );


            rendered.ownedModels.push_back(
                handle
            );


            eld::render::RenderObject
                object;


            object.model =
                handle;


            object.transform.position = {
                sceneOffsetX,
                0.0f,
                sceneOffsetZ
            };


            object.visible =
                true;


            rendered.objectSlots.push_back(
                placeObject(
                    std::move(
                        object
                    ),
                    scene
                )
            );
        }


        // Passthrough objects use handles supplied by
        // ModelSystem. Those models may be shared by multiple
        // Regions, so WorldScene must not destroy them.
        for (
            auto object :
            locationBatchBuild
                .passthroughObjects
        ) {
            object.transform.position.x +=
                sceneOffsetX;

            object.transform.position.z +=
                sceneOffsetZ;


            rendered.objectSlots.push_back(
                placeObject(
                    std::move(
                        object
                    ),
                    scene
                )
            );
        }


        renderedRegions_.emplace(
            region.id,
            std::move(
                rendered
            )
        );
    }


    void WorldScene::unloadRegion(
        std::uint16_t regionId,
        eld::render::RenderScene& scene
    )
    {
        const auto it =
            renderedRegions_.find(
                regionId
            );


        if (
            it ==
            renderedRegions_.end()
        ) {
            return;
        }


        auto& rendered =
            it->second;


        // ----------------------------------------------------
        // Release scene slots.
        //
        // DO NOT erase from scene.objects:
        // doing so would shift player/UI object indices.
        // ----------------------------------------------------

        for (
            const std::size_t slot :
            rendered.objectSlots
        ) {
            if (
                slot >=
                scene.objects.size()
            ) {
                continue;
            }


            scene.objects[
                slot
            ].visible =
                false;


            freeObjectSlots_.push_back(
                slot
            );
        }


        destroyOwnedModels(
            rendered
        );


        renderedRegions_.erase(
            it
        );
    }


    void WorldScene::buildLoadedRegions(
        const eld::world::World& world,
        const eld::world::TerrainOrigin& sceneOrigin,
        eld::render::RenderScene& scene
    )
    {
        for (
            const auto id :
            world.loadedRegionIds()
        ) {
            const auto* region =
                world.region(
                    id
                );


            if (!region) {
                continue;
            }


            loadRegion(
                *region,
                sceneOrigin,
                scene
            );
        }
    }


    void WorldScene::applyStreaming(
        const eld::world::World& world,
        const eld::runtime::map::WorldStreamingResult& streaming,
        const eld::world::TerrainOrigin& sceneOrigin,
        eld::render::RenderScene& scene
    )
    {
        // Runtime WorldStreamer has already loaded incoming
        // Regions and unloaded outgoing Regions when this is
        // called.
        //
        // On the render side we unload first so their object
        // slots can immediately be reused by incoming Regions.
        // This all happens between frames.

        for (
            const auto id :
            streaming.unloaded
        ) {
            unloadRegion(
                id,
                scene
            );
        }


        for (
            const auto id :
            streaming.loaded
        ) {
            const auto* region =
                world.region(
                    id
                );


            if (!region) {
                continue;
            }


            loadRegion(
                *region,
                sceneOrigin,
                scene
            );
        }
    }

}
