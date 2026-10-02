#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <deque>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#include <asio.hpp>

#include "Packet.h"
#include "PacketDecoder.h"


namespace eld::network
{

    class TcpClient
    {
    public:
        using PacketHandler =
            std::function<
                void(
                    const eld::protocol::Packet&
                )
            >;


        TcpClient();


        ~TcpClient();


        TcpClient(
            const TcpClient&
        ) = delete;


        TcpClient& operator=(
            const TcpClient&
        ) = delete;


        void setPacketHandler(
            PacketHandler handler
        );


        void connect(
            const std::string& host,
            std::uint16_t port
        );


        void send(
            std::vector<std::uint8_t> frame
        );


        bool connected() const noexcept;


    private:
        void read();


        void writeNext();


        asio::io_context
            io_;


        asio::ip::tcp::socket
            socket_;


        std::thread
            worker_;


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


        std::atomic_bool
            connected_ =
                false;
    };

}
