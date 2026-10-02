#pragma once

#include <cstdint>
#include <functional>
#include <memory>

#include <asio.hpp>


namespace eld::network
{

    class TcpSession;


    class TcpServer
    {
    public:
        using SessionHandler =
            std::function<
                void(
                    std::shared_ptr<TcpSession>
                )
            >;


        TcpServer(
            asio::io_context& context,
            std::uint16_t port,
            SessionHandler sessionHandler
        );


        void start();


        std::uint16_t port() const;


    private:
        void acceptNext();


        asio::ip::tcp::acceptor
            acceptor_;


        SessionHandler
            sessionHandler_;
    };

}
