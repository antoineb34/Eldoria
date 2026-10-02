#include "network/ClientSession.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>

#include "HelloPackets.h"
#include "Opcodes.h"
#include "PlayerPackets.h"
#include "ProtocolVersion.h"
#include "WalkPackets.h"


namespace eld::client
{

    ClientSession::ClientSession()
    {
        transport_.setPacketHandler(
            [this](
                const eld::protocol::Packet& packet
            )
            {
                handlePacket(
                    packet
                );
            }
        );
    }


    void ClientSession::connect(
        const std::string& host,
        std::uint16_t port
    )
    {
        transport_.connect(
            host,
            port
        );


        const eld::protocol::ClientHello hello{
            .protocolVersion =
                eld::protocol::ProtocolVersion
        };


        std::cout
            << "protocol | send ClientHello"
            << " | version="
            << hello.protocolVersion
            << "\n";


        transport_.send(
            eld::protocol::encode(
                hello
            )
        );
    }


    bool ClientSession::connected() const noexcept
    {
        return
            transport_.connected();
    }


    bool ClientSession::handshakeAccepted() const noexcept
    {
        return
            handshakeAccepted_.load();
    }


    void ClientSession::recordPrediction(
        const eld::world::TilePosition& source,
        const std::vector<
            eld::world::TilePosition
        >& path
    )
    {
        std::scoped_lock lock(
            stateMutex_
        );


        // First movement request:
        //
        // Until the server has explicitly reported a position,
        // the walk source is our known shared starting point.
        if (
            !authoritativePosition_.has_value()
        ) {
            authoritativePosition_ =
                source;
        }


        // ----------------------------------------------------
        // New path beginning at current authority:
        //
        // Replace all previous predicted movement.
        // ----------------------------------------------------

        if (
            *authoritativePosition_ ==
            source
        ) {
            predictedPositions_.clear();
        }
        else {
            // ------------------------------------------------
            // New click while another route is pending.
            //
            // Client PlayerController paths from its current
            // predicted destination.
            //
            // Server preserves its queue through that source
            // and replaces everything after it.
            //
            // Mirror that exact operation here.
            // ------------------------------------------------

            const auto sourceIterator =
                std::find(
                    predictedPositions_.begin(),
                    predictedPositions_.end(),
                    source
                );


            if (
                sourceIterator !=
                predictedPositions_.end()
            ) {
                predictedPositions_.erase(
                    std::next(
                        sourceIterator
                    ),
                    predictedPositions_.end()
                );
            }
            else {
                // We no longer understand how our prediction
                // relates to server authority.
                //
                // Keep the newly requested route, but the next
                // authoritative packet will become a correction
                // unless it matches.
                predictedPositions_.clear();
            }
        }


        for (
            const auto& tile :
            path
        ) {
            predictedPositions_.push_back(
                tile
            );
        }
    }


    void ClientSession::recordAuthoritativePosition(
        const eld::world::TilePosition& position
    )
    {
        std::scoped_lock lock(
            stateMutex_
        );


        PositionUpdateKind kind =
            PositionUpdateKind::Correction;


        // ----------------------------------------------------
        // Server may advance by one tile per tick now, or more
        // than one later when running.
        //
        // Therefore don't require the authoritative tile to be
        // exactly predictedPositions_.front().
        //
        // If it exists anywhere in the expected route, every
        // tile through it is now confirmed.
        // ----------------------------------------------------

        const auto confirmed =
            std::find(
                predictedPositions_.begin(),
                predictedPositions_.end(),
                position
            );


        if (
            confirmed !=
            predictedPositions_.end()
        ) {
            predictedPositions_.erase(
                predictedPositions_.begin(),
                std::next(
                    confirmed
                )
            );


            kind =
                PositionUpdateKind::Confirmation;
        }
        else if (
            predictedPositions_.empty() &&
            authoritativePosition_.has_value() &&
            *authoritativePosition_ ==
                position
        ) {
            // Repeated authoritative state while nothing is
            // predicted is harmless.
            kind =
                PositionUpdateKind::Confirmation;
        }
        else if (
            predictedPositions_.empty() &&
            !authoritativePosition_.has_value()
        ) {
            // Initial authoritative state.
            kind =
                PositionUpdateKind::Confirmation;
        }
        else {
            // Server reported something outside the route we
            // predicted.
            //
            // Our local prediction can no longer be trusted.
            predictedPositions_.clear();
        }


        authoritativePosition_ =
            position;


        positionUpdates_.push_back({
            .position =
                position,

            .kind =
                kind
        });
    }


    std::vector<
        AuthoritativePositionUpdate
    >
    ClientSession::takePositionUpdates()
    {
        std::scoped_lock lock(
            stateMutex_
        );


        std::vector<
            AuthoritativePositionUpdate
        >
            result;


        result.reserve(
            positionUpdates_.size()
        );


        while (
            !positionUpdates_.empty()
        ) {
            result.push_back(
                std::move(
                    positionUpdates_.front()
                )
            );


            positionUpdates_.pop_front();
        }


        return result;
    }


    void ClientSession::handlePacket(
        const eld::protocol::Packet& packet
    )
    {
        using eld::protocol::ServerOpcode;


        switch (
            static_cast<ServerOpcode>(
                packet.opcode
            )
        ) {
            case ServerOpcode::Hello:
            {
                const auto hello =
                    eld::protocol::decodeServerHello(
                        packet
                    );


                handshakeAccepted_.store(
                    hello.accepted
                );


                std::cout
                    << "protocol | recv ServerHello"
                    << " | accepted="
                    << (
                        hello.accepted
                            ? "true"
                            : "false"
                    )
                    << " | version="
                    << hello.protocolVersion
                    << "\n";


                break;
            }


            case ServerOpcode::PlayerPosition:
            {
                const auto packetPosition =
                    eld::protocol::decodePlayerPosition(
                        packet
                    );


                const eld::world::TilePosition
                    position{
                        static_cast<int>(
                            packetPosition.x
                        ),

                        static_cast<int>(
                            packetPosition.y
                        ),

                        static_cast<int>(
                            packetPosition.plane
                        )
                    };


                recordAuthoritativePosition(
                    position
                );


                std::cout
                    << "protocol | recv PlayerPosition"
                    << " | "
                    << position.x
                    << ","
                    << position.y
                    << ","
                    << position.plane
                    << "\n";


                break;
            }


            default:
                std::cout
                    << "protocol | unknown server opcode | "
                    << static_cast<unsigned>(
                        packet.opcode
                    )
                    << "\n";

                break;
        }
    }


    void ClientSession::sendWalkRequest(
        const eld::world::TilePosition& source,
        const std::vector<
            eld::world::TilePosition
        >& path,
        bool running
    )
    {
        std::cout
            << "protocol | enter WalkRequest"
            << " | connected="
            << (
                connected()
                    ? "true"
                    : "false"
            )
            << " | handshake="
            << (
                handshakeAccepted()
                    ? "true"
                    : "false"
            )
            << " | path="
            << path.size()
            << "\n";


        if (
            !handshakeAccepted()
        ) {
            std::cout
                << "protocol | WalkRequest skipped"
                << " | handshake not accepted"
                << "\n";

            return;
        }


        if (
            path.empty()
        ) {
            return;
        }


        constexpr int U16Max =
            std::numeric_limits<
                std::uint16_t
            >::max();


        if (
            source.x < 0 ||
            source.x > U16Max ||
            source.y < 0 ||
            source.y > U16Max ||
            source.plane < 0 ||
            source.plane > 255
        ) {
            throw std::runtime_error(
                "WalkRequest source lies outside protocol range"
            );
        }


        eld::protocol::WalkRequest
            request{
                .sourceX =
                    static_cast<std::uint16_t>(
                        source.x
                    ),

                .sourceY =
                    static_cast<std::uint16_t>(
                        source.y
                    ),

                .plane =
                    static_cast<std::uint8_t>(
                        source.plane
                    ),

                .running =
                    running
            };


        request.steps.reserve(
            path.size()
        );


        auto previous =
            source;


        for (
            const auto& tile :
            path
        ) {
            if (
                tile.plane !=
                previous.plane
            ) {
                throw std::runtime_error(
                    "WalkRequest changes plane inside path"
                );
            }


            const int dx =
                tile.x -
                previous.x;


            const int dy =
                tile.y -
                previous.y;


            if (
                dx < -1 ||
                dx > 1 ||
                dy < -1 ||
                dy > 1 ||
                (
                    dx == 0 &&
                    dy == 0
                )
            ) {
                throw std::runtime_error(
                    "WalkRequest path contains non-adjacent step"
                );
            }


            request.steps.push_back({
                .dx =
                    static_cast<std::int8_t>(
                        dx
                    ),

                .dy =
                    static_cast<std::int8_t>(
                        dy
                    )
            });


            previous =
                tile;
        }


        // Record what we expect the authoritative server to
        // report BEFORE putting the request onto the network.
        //
        // That guarantees a very fast response cannot race
        // ahead of our prediction bookkeeping.
        recordPrediction(
            source,
            path
        );


        std::cout
            << "protocol | send WalkRequest"
            << " | source="
            << request.sourceX
            << ","
            << request.sourceY
            << ","
            << static_cast<unsigned>(
                request.plane
            )
            << " | destination="
            << previous.x
            << ","
            << previous.y
            << ","
            << previous.plane
            << " | steps="
            << request.steps.size()
            << " | running="
            << (
                running
                    ? "true"
                    : "false"
            )
            << "\n";


        auto frame =
            eld::protocol::encode(
                request
            );


        std::cout
            << "protocol | dispatch WalkRequest"
            << " | bytes="
            << frame.size()
            << "\n";


        transport_.send(
            std::move(
                frame
            )
        );
    }

}
