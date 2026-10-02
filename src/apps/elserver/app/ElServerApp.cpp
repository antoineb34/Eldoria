#include "ElServerApp.h"

#include "ServerRunLoop.h"

#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <asio.hpp>

#include "TcpServer.h"
#include "assets/AssetManager.h"
#include "game/ServerWorld.h"
#include "session/Session.h"
#include "world/Position.h"


namespace eld::elserver
{

    int ElServerApp::run()
    {
        try {
            constexpr std::uint16_t
                GamePort =
                    43594;


            const eld::world::TilePosition
                Spawn{
                    3232,
                    3232,
                    0
                };


            // Server owns its own decoded Region instances.
            eld::asset::AssetManager
                assets;


            ServerWorld
                world(
                    assets.maps,
                    assets.locations
                );


            // Bootstrap only.
            //
            // This is NOT client-style player streaming.
            // Later the server can load/cache regions using
            // whatever policy we want.
            world.loadAround(
                Spawn,
                1
            );


            if (
                !world.regionAt(
                    Spawn
                )
            ) {
                throw std::runtime_error(
                    "Server spawn region could not be loaded"
                );
            }


            std::cout
                << "server world | ready | regions="
                << world.loadedRegionCount()
                << "\n";


            asio::io_context
                io;


            ServerRunLoop
                runLoop(
                    io
                );


            eld::network::TcpServer
                server(
                    io,
                    GamePort,

                    [
                        &world,
                        &runLoop
                    ](
                        std::shared_ptr<
                            eld::network::TcpSession
                        > transport
                    )
                    {
                        auto session =
                            Session::attach(
                                transport,
                                world
                            );


                        runLoop.addSession(
                            session
                        );
                    }
                );


            std::cout
                << "ElServer starting...\n";


            server.start();


            runLoop.start();


            io.run();


            std::cout
                << "ElServer shutdown.\n";


            return 0;
        }
        catch (
            const std::exception& error
        ) {
            std::cerr
                << "ElServer fatal | "
                << error.what()
                << "\n";


            return 1;
        }
    }

}
