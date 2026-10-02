#include "session/Session.h"

#include <algorithm>

#include <iostream>
#include <memory>
#include <vector>

#include "HelloPackets.h"
#include "Opcodes.h"
#include "ProtocolVersion.h"
#include "PlayerPackets.h"
#include "TcpSession.h"
#include "WalkPackets.h"
#include "game/ServerWorld.h"


namespace eld::elserver
{

    Session::Session(
        const std::shared_ptr<
            eld::network::TcpSession
        >& transport,

        ServerWorld& world
    )
        : transport_(
              transport
          ),
          world_(
              world
          )
    {
    }


    std::shared_ptr<Session>
    Session::attach(
        const std::shared_ptr<
            eld::network::TcpSession
        >& transport,

        ServerWorld& world
    )
    {
        auto session =
            std::shared_ptr<Session>(
                new Session(
                    transport,
                    world
                )
            );


        transport->setPacketHandler(
            [
                session
            ](
                const eld::protocol::Packet& packet
            )
            {
                session->handlePacket(
                    packet
                );
            }
        );


        return session;
    }


    void Session::handlePacket(
        const eld::protocol::Packet& packet
    )
    {
        using eld::protocol::ClientOpcode;


        if (
            state_ ==
            State::Handshake
        ) {
            if (
                packet.opcode !=
                static_cast<std::uint8_t>(
                    ClientOpcode::Hello
                )
            ) {
                std::cout
                    << "session | expected ClientHello"
                    << " | opcode="
                    << static_cast<unsigned>(
                        packet.opcode
                    )
                    << "\n";


                return;
            }


            handleHello(
                packet
            );


            return;
        }


        switch (
            static_cast<ClientOpcode>(
                packet.opcode
            )
        ) {
            case ClientOpcode::Hello:
                std::cout
                    << "session | duplicate ClientHello ignored"
                    << "\n";

                break;


            case ClientOpcode::Walk:
                handleWalk(
                    packet
                );

                break;


            default:
                std::cout
                    << "session | unhandled client opcode | "
                    << static_cast<unsigned>(
                        packet.opcode
                    )
                    << "\n";

                break;
        }
    }


    void Session::handleHello(
        const eld::protocol::Packet& packet
    )
    {
        const auto hello =
            eld::protocol::decodeClientHello(
                packet
            );


        const bool accepted =
            hello.protocolVersion ==
            eld::protocol::ProtocolVersion;


        std::cout
            << "protocol | recv ClientHello"
            << " | version="
            << hello.protocolVersion
            << "\n";


        const eld::protocol::ServerHello reply{
            .accepted =
                accepted,

            .protocolVersion =
                eld::protocol::ProtocolVersion
        };


        std::cout
            << "protocol | send ServerHello"
            << " | accepted="
            << (
                accepted
                    ? "true"
                    : "false"
            )
            << " | version="
            << reply.protocolVersion
            << "\n";


        if (
            auto transport =
                transport_.lock()
        ) {
            transport->send(
                eld::protocol::encode(
                    reply
                )
            );
        }


        if (
            !accepted
        ) {
            return;
        }


        player_ =
            std::make_unique<Player>();


        if (
            !world_.regionAt(
                player_->position
            )
        ) {
            std::cerr
                << "session | spawn region unavailable"
                << " | position="
                << player_->position.x
                << ","
                << player_->position.y
                << ","
                << player_->position.plane
                << "\n";


            player_.reset();


            return;
        }


        state_ =
            State::Connected;


        std::cout
            << "session | connected"
            << " | player="
            << player_->id
            << " | position="
            << player_->position.x
            << ","
            << player_->position.y
            << ","
            << player_->position.plane
            << "\n";
    }


    void Session::handleWalk(
        const eld::protocol::Packet& packet
    )
    {
        if (
            !player_
        ) {
            std::cout
                << "movement | rejected"
                << " | no player"
                << "\n";

            return;
        }


        const auto request =
            eld::protocol::decodeWalkRequest(
                packet
            );


        const eld::world::TilePosition source{
            static_cast<int>(
                request.sourceX
            ),
            static_cast<int>(
                request.sourceY
            ),
            static_cast<int>(
                request.plane
            )
        };


        std::cout
            << "protocol | recv WalkRequest"
            << " | source="
            << source.x
            << ","
            << source.y
            << ","
            << source.plane
            << " | steps="
            << request.steps.size()
            << " | running="
            << (
                request.running
                    ? "true"
                    : "false"
            )
            << "\n";


        std::size_t preserveCount =
            0;


        if (
            source !=
            player_->position
        ) {
            const auto iterator =
                std::find(
                    player_
                        ->movementQueue
                        .begin(),

                    player_
                        ->movementQueue
                        .end(),

                    source
                );


            if (
                iterator ==
                player_
                    ->movementQueue
                    .end()
            ) {
                std::cout
                    << "movement | rejected"
                    << " | source mismatch"
                    << " | server="
                    << player_->position.x
                    << ","
                    << player_->position.y
                    << ","
                    << player_->position.plane
                    << "\n";


                return;
            }


            preserveCount =
                static_cast<std::size_t>(
                    std::distance(
                        player_
                            ->movementQueue
                            .begin(),

                        iterator
                    )
                ) +
                1;
        }


        std::vector<
            eld::world::TilePosition
        >
            validatedPath;


        validatedPath.reserve(
            request.steps.size()
        );


        auto current =
            source;


        for (
            std::size_t index = 0;
            index <
                request.steps.size();
            ++index
        ) {
            const auto& step =
                request.steps[index];


            const eld::world::TilePosition next{
                current.x +
                    static_cast<int>(
                        step.dx
                    ),

                current.y +
                    static_cast<int>(
                        step.dy
                    ),

                current.plane
            };


            if (
                !world_.canMove(
                    current,
                    next
                )
            ) {
                std::cout
                    << "movement | rejected"
                    << " | blocked step="
                    << index
                    << " | from="
                    << current.x
                    << ","
                    << current.y
                    << " | to="
                    << next.x
                    << ","
                    << next.y
                    << "\n";


                return;
            }


            validatedPath.push_back(
                next
            );


            current =
                next;
        }


        while (
            player_
                ->movementQueue
                .size() >
            preserveCount
        ) {
            player_
                ->movementQueue
                .pop_back();
        }


        for (
            const auto& tile :
            validatedPath
        ) {
            player_
                ->movementQueue
                .push_back(
                    tile
                );
        }


        player_->running =
            request.running;


        std::cout
            << "movement | queued"
            << " | destination="
            << current.x
            << ","
            << current.y
            << ","
            << current.plane
            << " | steps="
            << player_
                ->movementQueue
                .size()
            << "\n";
    }


    void Session::tick()
    {
        if (
            state_ !=
                State::Connected ||
            !player_ ||
            player_
                ->movementQueue
                .empty()
        ) {
            return;
        }


        const int stepCount =
            player_->running
                ? 2
                : 1;


        bool moved =
            false;


        for (
            int step = 0;
            step < stepCount;
            ++step
        ) {
            if (
                player_
                    ->movementQueue
                    .empty()
            ) {
                break;
            }


            player_->position =
                player_
                    ->movementQueue
                    .front();


            player_
                ->movementQueue
                .pop_front();


            moved =
                true;
        }


        if (
            !moved
        ) {
            return;
        }


        std::cout
            << "server tick | player="
            << player_->id
            << " | position="
            << player_->position.x
            << ","
            << player_->position.y
            << ","
            << player_->position.plane
            << " | remaining="
            << player_
                ->movementQueue
                .size()
            << "\n";


        if (
            auto transport =
                transport_.lock()
        ) {
            const eld::protocol::PlayerPosition
                position{
                    .x =
                        static_cast<std::uint16_t>(
                            player_->position.x
                        ),

                    .y =
                        static_cast<std::uint16_t>(
                            player_->position.y
                        ),

                    .plane =
                        static_cast<std::uint8_t>(
                            player_->position.plane
                        )
                };


            transport->send(
                eld::protocol::encode(
                    position
                )
            );
        }
    }

}
