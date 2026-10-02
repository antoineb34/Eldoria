#pragma once

#include <array>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <vector>

#include <asio.hpp>

#include "Packet.h"
#include "PacketDecoder.h"


namespace eld::network
{

    class TcpSession :
        public std::enable_shared_from_this<
            TcpSession
        >
    {
    public:
        using PacketHandler =
            std::function<
                void(
                    const eld::protocol::Packet&
                )
            >;


        explicit TcpSession(
            asio::ip::tcp::socket socket
        );


        void start();


        void setPacketHandler(
            PacketHandler handler
        );


        void send(
            std::vector<std::uint8_t> frame
        );


    private:
        void read();


        void writeNext();


        asio::ip::tcp::socket
            socket_;


        std::array<std::uint8_t, 4096>
            readBuffer_{};


        std::deque<
            std::vector<std::uint8_t>
        >
            writeQueue_;


        eld::protocol::PacketDecoder
            decoder_;


        PacketHandler
            packetHandler_;
    };

}
