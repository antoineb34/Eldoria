#include "WorldStreamer.h"

#include <exception>
#include <utility>


namespace eld::runtime::map
{

    WorldStreamer::WorldStreamer(
        eld::world::World& world,
        const eld::map::MapLoader& maps,
        const eld::location::LocationLoader& locations
    )
        : world_(
              world
          ),
          provider_(
              maps,
              locations
          )
    {
    }


    WorldStreamingResult
    WorldStreamer::update(
        const eld::world::TilePosition& player
    )
    {
        WorldStreamingResult
            result;


        result.plan =
            world_.updateStreaming(
                player
            );


        // Load first so the incoming streaming window exists
        // before anything from the previous window disappears.
        for (
            const auto id :
            result.plan.toLoad
        ) {
            try {
                world_.insertRegion(
                    provider_.load(
                        id
                    )
                );


                result.loaded.push_back(
                    id
                );
            }
            catch (
                const std::exception& error
            ) {
                result.failures.push_back({
                    id,
                    error.what()
                });
            }
        }


        for (
            const auto id :
            result.plan.toUnload
        ) {
            if (
                world_.unloadRegion(
                    id
                )
            ) {
                result.unloaded.push_back(
                    id
                );
            }
        }


        return result;
    }

}
