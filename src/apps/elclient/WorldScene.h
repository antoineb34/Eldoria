#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "floor/FloorLoader.h"
#include "location/LocationLoader.h"
#include "map/WorldStreamer.h"
#include "model/ModelLoader.h"
#include "model/ModelSystem.h"
#include "render/model/ModelManager.h"
#include "render/scene/RenderScene.h"
#include "texture/TextureSystem.h"
#include "world/Region.h"
#include "world/World.h"


namespace eld::client
{

    class WorldScene
    {
    public:
        WorldScene(
            const eld::floor::FloorLoader& floors,
            const eld::location::LocationLoader& locations,
            eld::model::ModelLoader& models,
            eld::graphics::TextureSystem& textureSystem,
            eld::graphics::ModelSystem& modelSystem,
            eld::render::ModelManager& modelManager
        );


        ~WorldScene();


        void reset();


        void buildLoadedRegions(
            const eld::world::World& world,
            const eld::world::TerrainOrigin& sceneOrigin,
            eld::render::RenderScene& scene
        );


        void applyStreaming(
            const eld::world::World& world,
            const eld::runtime::map::WorldStreamingResult& streaming,
            const eld::world::TerrainOrigin& sceneOrigin,
            eld::render::RenderScene& scene
        );


        std::size_t renderedRegionCount() const noexcept;


    private:
        struct RenderedRegion
        {
            std::vector<std::size_t>
                objectSlots;

            // Terrain and location batches are created
            // specifically for this rendered Region.
            //
            // Passthrough location models come from ModelSystem
            // and are shared, so they are NOT owned here.
            std::vector<eld::render::ModelHandle>
                ownedModels;
        };


        std::size_t placeObject(
            eld::render::RenderObject object,
            eld::render::RenderScene& scene
        );


        void loadRegion(
            const eld::world::Region& region,
            const eld::world::TerrainOrigin& sceneOrigin,
            eld::render::RenderScene& scene
        );


        void unloadRegion(
            std::uint16_t regionId,
            eld::render::RenderScene& scene
        );


        void destroyOwnedModels(
            RenderedRegion& rendered
        );


        const eld::floor::FloorLoader&
            floors_;

        const eld::location::LocationLoader&
            locations_;

        eld::model::ModelLoader&
            models_;

        eld::graphics::TextureSystem&
            textureSystem_;

        eld::graphics::ModelSystem&
            modelSystem_;

        eld::render::ModelManager&
            modelManager_;


        std::unordered_map<
            std::uint16_t,
            RenderedRegion
        > renderedRegions_;


        // Region objects may be removed while player/UI objects
        // remain in RenderScene. Reusing only region-owned slots
        // keeps all unrelated RenderScene indices stable.
        std::vector<std::size_t>
            freeObjectSlots_;
    };

}
