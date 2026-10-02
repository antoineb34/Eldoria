#pragma once

#include <memory>

#include "Packet.h"
#include "game/Player.h"


namespace eld::network
{

    class TcpSession;

}


namespace eld::elserver
{

    class ServerWorld;


    class Session
    {
    public:
        static std::shared_ptr<Session> attach(
            const std::shared_ptr<
                eld::network::TcpSession
            >& transport,

            ServerWorld& world
        );


        void tick();


    private:
        enum class State
        {
            Handshake,
            Connected
        };


        Session(
            const std::shared_ptr<
                eld::network::TcpSession
            >& transport,

            ServerWorld& world
        );


        void handlePacket(
            const eld::protocol::Packet& packet
        );


        void handleHello(
            const eld::protocol::Packet& packet
        );


        void handleWalk(
            const eld::protocol::Packet& packet
        );


        std::weak_ptr<
            eld::network::TcpSession
        >
            transport_;


        ServerWorld&
            world_;


        State
            state_ =
                State::Handshake;


        std::unique_ptr<Player>
            player_;
    };

}
