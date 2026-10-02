#pragma once

#include <atomic>
#include <cstdint>
#include <deque>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "TcpClient.h"
#include "world/Position.h"


namespace eld::client
{

    enum class PositionUpdateKind
    {
        Confirmation,
        Correction
    };


    struct AuthoritativePositionUpdate
    {
        eld::world::TilePosition
            position{};


        PositionUpdateKind
            kind =
                PositionUpdateKind::Confirmation;
    };


    class ClientSession
    {
    public:
        ClientSession();


        void connect(
            const std::string& host,
            std::uint16_t port
        );


        void sendWalkRequest(
            const eld::world::TilePosition& source,
            const std::vector<
                eld::world::TilePosition
            >& path,
            bool running
        );


        std::vector<
            AuthoritativePositionUpdate
        >
        takePositionUpdates();


        bool connected() const noexcept;


        bool handshakeAccepted() const noexcept;


    private:
        void handlePacket(
            const eld::protocol::Packet& packet
        );


        void recordPrediction(
            const eld::world::TilePosition& source,
            const std::vector<
                eld::world::TilePosition
            >& path
        );


        void recordAuthoritativePosition(
            const eld::world::TilePosition& position
        );


        eld::network::TcpClient
            transport_;


        std::atomic_bool
            handshakeAccepted_ =
                false;


        mutable std::mutex
            stateMutex_;


        std::optional<
            eld::world::TilePosition
        >
            authoritativePosition_;


        std::deque<
            eld::world::TilePosition
        >
            predictedPositions_;


        std::deque<
            AuthoritativePositionUpdate
        >
            positionUpdates_;
    };

}
